// using peglib to parse vnnlib file;

#ifndef NEURAL_NETWORK_PARSER_HPP
#define NEURAL_NETWORK_PARSER_HPP

#include "peglib.h"
#include <assert.h>
#include <iostream>
#include <stdexcept>

#include "ast.hpp"
#include "layer.hpp"
#include "specifications.hpp"

#include "../dep/onnx-1.15.0/onnx.proto3.pb.h"
#include "fstream"

class NeuralNetworkParser {
  using SV = peg::SemanticValues;

public:
  bool load_network(const std::string &network_file_directory,
                    const Specification &spec, std::vector<Layer> &layers) {
    const std::string model_path = network_file_directory;

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    onnx::ModelProto model;
    std::ifstream input(model_path, std::ios::in | std::ios::binary);

    if (!input) {
      std::cerr << "Failed to open: " << model_path << std::endl;
      return false;
    }

    if (!model.ParseFromIstream(&input)) {
      std::cerr << "Failed to parse ONNX file." << std::endl;
      return false;
    }

    const onnx::GraphProto &graph = model.graph();
    std::cout << "Model name: " << graph.name() << "\n";
    std::cout << "Number of nodes: " << graph.node_size() << "\n";
    std::cout << "Number of initializers (weights/biases): "
              << graph.initializer_size() << "\n\n";

    // --- Create a map from tensor name to TensorProto for fast lookup ---
    std::unordered_map<std::string, onnx::TensorProto> tensor_map;
    for (const auto &initializer : graph.initializer()) {
      tensor_map[initializer.name()] = initializer;
    }

    // --- Iterate over nodes (layers) ---
    for (const auto &node : graph.node()) {
      std::cout << "Node: " << node.name() << " | OpType: " << node.op_type()
                << "\n";

      Layer layer;
      if (node.op_type() == "Sub") {
        layer.type = LayerType::Sub;
      } else if (node.op_type() == "Flatten") {
        layer.type = LayerType::Flatten;
      } else if (node.op_type() == "MatMul") {
        layer.type = LayerType::MatMul;
      } else if (node.op_type() == "Add") {
        layer.type = LayerType::Add;
      } else if (node.op_type() == "Gemm") {
        layer.type = LayerType::Gemm;
      } else if (node.op_type() == "Relu") {
        layer.type = LayerType::Relu;
      }

      // TODO: modify the rule to identify differnet types of layers.
      // Print inputs and outputs
      for (const auto &input_name : node.input()) {
        std::cout << "  Input: " << input_name << "\n";

        // If the input is a weight tensor
        if (tensor_map.find(input_name) != tensor_map.end()) {
          const auto &tensor = tensor_map[input_name];
          if (tensor.dims().size() == 1) {
            std::vector<float> biases = extract1DTensorData(tensor);
            layer.biases = biases;
            layer.lower_biases = biases;
            layer.upper_biases = biases;
            layer.layer_size = biases.size();
            std::cout << " (bias tensor, size = " << biases.size() << ")\n";
          } else if (tensor.dims().size() == 2) {
            std::vector<std::vector<float>> weights =
                extract2DTensorData(tensor);
            layer.weights = weights;
            // (output_dimension, input_dimension);
            layer.layer_size = weights[0].size();
            std::cout << " (weight tensor, size = (" << weights.size() << ", "
                      << weights[0].size() << ")" << ")\n";
          }
          // layer.neurons = new Neuron[layer.layer_size];
          layer.neurons = std::vector<Neuron>(layer.layer_size);
        } else if (layer.type == LayerType::Sub) {
          std::cout << "This is a subtraction layer.\n";
          continue;
        } else if (layer.type == LayerType::Flatten) {
          std::cout << "This is a flatten layer.\n";
          for (const auto &variable : spec.variables) {
            if (variable.first.substr(0, 1) == "X") {
              // show the preconditions
              std::cout << variable.second.id << ": ["
                        << variable.second.bounds.getLb() << ", "
                        << variable.second.bounds.getUb() << "]\n";

              layer.neurons.push_back(variable.second);
            }
          }
          layer.layer_size = layer.neurons.size();
        } else if (layer.type == LayerType::Relu) {
          layer.layer_size = layers[layers.size() - 1].layer_size;
          layer.neurons = std::vector<Neuron>(layer.layer_size);
          std::cout << "This is an acitvation layer.\n";
        }
      }

      for (const auto &output_name : node.output()) {
        std::cout << "  Output: " << output_name << "\n";
      }

      // update neuron index and layer index
      for (size_t i = 0; i < layer.layer_size; ++i) {
        layer.neurons[i].setId(i);
        layer.neurons[i].setLayerId(layers.size());
      }

      layers.push_back(layer);
      std::cout << "---------------------------------------\n";
    }

    google::protobuf::ShutdownProtobufLibrary();

    return true;
  }

  Specification parse(const std::string &input) {
    peg::parser parser(R"(
                            Specifications          <- Statements* 
                            Statements              <- DeclareVar / Assertion / Comment
                            Integer                 <- [+-]? [0-9]+
                            Float                   <- [+-]? [0-9]+ '.' [0-9]+
                            Identifier              <- [a-zA-Z_][a-zA-Z0-9_]*
                            BinaryOp                <- '<=' / '>='
                            LogicOp                 <- 'or' / 'and'
                            DeclareVar              <- '(declare-const '  Identifier ' Real' ')'
                            Bound                   <- '(' BinaryOp [ \t]* Identifier [ \t]* (Float / Identifier) [ \t]* ')'
                            Conjunctive             <- '(' LogicOp [ \t]* Bound* ')' 
                            Assertion               <- '(assert ' Bound ')' / '(assert (' LogicOp+ Conjunctive* '))' 
                            ~Comment                <- ';' [^\n\r]* [ \n\r\t]*
                            %whitespace             <- [ \n\r\t]*
                           )");
    assert(static_cast<bool>(parser) == true);

    // setup actions
    parser["Specifications"] = [this](const SV &sv) {
      return make_specifications(sv);
    };
    parser["Integer"] = [](const SV &sv) {
      return ASTNode(sv.token_to_number<int>());
    };
    parser["Float"] = [](const SV &sv) {
      return ASTNode(sv.token_to_number<double>());
    };
    parser["Identifier"] = [](const SV &sv) {
      return ASTNode(sv.token_to_string());
    };
    parser["BinaryOp"] = [this](const SV &sv) { return make_binary_op(sv); };
    parser["LogicOp"] = [this](const SV &sv) { return make_logic_op(sv); };
    parser["DeclareVar"] = [this](const SV &sv) {
      return make_declare_variable(sv);
    };
    parser["Bound"] = [this](const SV &sv) { return make_bound(sv); };
    parser["Conjunctive"] = [this](const SV &sv) {
      return make_conjunctive(sv);
    };
    parser["Assertion"] = [this](const SV &sv) { return make_assertion(sv); };
    parser.set_logger([](size_t line, size_t col, const std::string &msg,
                         const std::string &rule) {
      std::cerr << "Error at line " << line << ", column " << col << ": " << msg
                << " in rule " << rule << std::endl;
    });

    std::cout << input.c_str() << std::endl;
    ASTNode ast;
    if (parser.parse(input.c_str(), ast)) {
      std::cout << "Parsing succeeded!" << std::endl;
    } else {
      std::cerr << "Parsing failed!" << std::endl;
    }

    Specification spec;
    ast.make_specifications(spec);
    ast.print_bounds(spec);
    return spec;
  }

private:
  ASTNode make_specifications(const SV &sv) {
    if (sv.size() == 1) {
      return std::any_cast<ASTNode>(sv[0]);
    } else {
      ASTNode root;
      for (size_t i = 0; i < sv.size(); ++i) {
        try {
          root.children.push_back(std::any_cast<ASTNode>(sv[i]));
        } catch (const std::bad_any_cast &e) {
          // std::cerr << "Bad any cast: " << e.what() << std::endl;
          continue;
        }
      }
      return root;
    }
  }

  ASTNode make_binary_op(const SV &sv) {
    ASTNode bop_node(ASTNodeType::BINARY_OP);
    std::string bop = sv.token_to_string();
    if (bop == "<=")
      bop_node.value = BinaryOp::LessEqual;
    else if (bop == ">=")
      bop_node.value = BinaryOp::GreaterEqual;
    else
      throw std::runtime_error("Unknown binary operator: " + bop);
    return bop_node;
  }

  ASTNode make_logic_op(const SV &sv) {
    ASTNode lop_node(ASTNodeType::LOGIC_OP);
    std::string lop = sv.token_to_string();
    if (lop == "and")
      lop_node.value = LogicOp::And;
    else if (lop == "or")
      lop_node.value = LogicOp::Or;
    else
      throw std::runtime_error("Unknown logic operator: " + lop);
    return lop_node;
  }

  ASTNode make_declare_variable(const SV &sv) {
    ASTNode decl_node(ASTNodeType::DECLARATION, std::string("double"));
    for (size_t i = 0; i < sv.size(); ++i) {
      decl_node.children.push_back(std::any_cast<ASTNode>(sv[i]));
    }
    return decl_node;
  }

  ASTNode make_bound(const SV &sv) {
    ASTNode bound_node(ASTNodeType::BOUND);
    ASTNode lop_node = std::any_cast<ASTNode>(sv[0]);
    ASTNode left = std::any_cast<ASTNode>(sv[1]);
    ASTNode right = std::any_cast<ASTNode>(sv[2]);

    lop_node.children.push_back(left);
    lop_node.children.push_back(right);
    bound_node.children.push_back(lop_node);

    return bound_node;
  }

  ASTNode make_conjunctive(const SV &sv) {
    ASTNode conj_node(ASTNodeType::LOGIC_OP, LogicOp::And); // sv[0];
    for (size_t i = 1; i < sv.size(); ++i) {
      ASTNode bound_node = std::any_cast<ASTNode>(sv[i]);
      conj_node.children.push_back(bound_node);
    }

    return conj_node;
  }

  ASTNode make_assertion(const SV &sv) {
    ASTNode assert_node(ASTNodeType::ASSERTION);
    if (sv.size() == 1) {
      ASTNode bound_node = std::any_cast<ASTNode>(sv[0]);
      assert_node.children.push_back(bound_node);
    } else {
      // TODO: modify
      for (size_t i = 0; i < sv.size(); ++i) {
        ASTNode node = std::any_cast<ASTNode>(sv[i]);
        assert_node.children.push_back(node);
      }
    }

    return assert_node;
  }

  std::vector<float> extract1DTensorData(const onnx::TensorProto &tensor) {
    std::vector<float> data;

    // Case 1: float_data() directly stored
    if (tensor.float_data_size() > 0) {
      data.assign(tensor.float_data().begin(), tensor.float_data().end());
    }
    // Case 2: raw_data() binary blob
    else {
      std::string raw = tensor.raw_data();
      size_t elem_count = raw.size() / sizeof(float);
      data.resize(elem_count);
      std::memcpy(data.data(), raw.data(), raw.size());
    }

    return data;
  }

  std::vector<std::vector<float>>
  extract2DTensorData(const onnx::TensorProto &tensor) {
    std::vector<std::vector<float>> data;

    std::vector<float> temp_data;

    // Case 1: float_data() directly stored
    if (tensor.float_data_size() > 0) {
      temp_data.assign(tensor.float_data().begin(), tensor.float_data().end());
    }
    // Case 2: raw_data() binary blob
    else {
      std::string raw = tensor.raw_data();
      size_t elem_count = raw.size() / sizeof(float);
      temp_data.resize(elem_count);
      std::memcpy(temp_data.data(), raw.data(), raw.size());
    }

    // make 1d data vector to 2d data matrix;
    size_t num_rows = tensor.dims(0);
    size_t num_cols = tensor.dims(1);
    data.resize(num_rows, std::vector<float>(num_cols));
    for (size_t r = 0; r < num_rows; ++r) {
      for (size_t c = 0; c < num_cols; ++c) {
        data[r][c] = temp_data[r * num_cols + c];
      }
    }

    return data;
  }
};

#endif // NEURAL_NETWORK_PARSER_HPP
