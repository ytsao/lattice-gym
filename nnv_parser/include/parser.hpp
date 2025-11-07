// using peglib to parse vnnlib file;

#ifndef NEURAL_NETWORK_PARSER_HPP
#define NEURAL_NETWORK_PARSER_HPP

#include "peglib.h"
#include <assert.h>
#include <cmath>
#include <stdexcept>

#include "ast.hpp"
#include "layer.hpp"
#include "log.hpp"
#include "specifications.hpp"

#include "../dep/onnx-1.15.0/onnx.proto3.pb.h"
#include "fstream"

class NeuralNetworkParser
{
  using SV = peg::SemanticValues;

  using tensor1d = std::vector<float>;
  using tensor2d = std::vector<tensor1d>;
  using tensor3d = std::vector<tensor2d>;
  using tensor4d = std::vector<tensor3d>;

public:
  bool load_network(const std::string &network_file_directory,
                    const Specification &spec, std::vector<Layer> &layers)
  {
    const std::string model_path = network_file_directory;

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    onnx::ModelProto model;
    std::ifstream input(model_path, std::ios::in | std::ios::binary);

    if (!input)
    {
      Logger::log(Logger::Level::ERROR, "Failed to open: " + model_path);
      return false;
    }

    if (!model.ParseFromIstream(&input))
    {
      Logger::log(Logger::Level::ERROR, "Failed to parse ONNX file.");
      return false;
    }

    const onnx::GraphProto &graph = model.graph();
    Logger::log(Logger::Level::INFO, "Model name: " + graph.name());
    Logger::log(Logger::Level::INFO,
                "Number of nodes: " + std::to_string(graph.node_size()));
    Logger::log(Logger::Level::INFO,
                "Number of initializers (weights/biases): " +
                    std::to_string(graph.initializer_size()));

    // --- Create a map from tensor name to TensorProto for fast lookup ---
    std::unordered_map<std::string, onnx::TensorProto> tensor_map;
    for (const auto &initializer : graph.initializer())
    {
      tensor_map[initializer.name()] = initializer;
      Logger::log(Logger::Level::DEBUG, initializer.name());
    }

    // shape of input.
    const onnx::ValueInfoProto &graph_input = graph.input(0);
    const auto &input_shape =
        graph_input.type()
            .tensor_type()
            .shape(); // <batch_size, num_channels, input_H, input-W);
    size_t batch_size = input_shape.dim(0).dim_value();
    size_t input_channels = input_shape.dim(1).dim_value();
    size_t input_height = input_shape.dim(2).dim_value();
    size_t input_width = input_shape.dim(3).dim_value();
    size_t input_dimension =
        batch_size * input_channels * input_height * input_width;
    assert(input_dimension == spec.numberOfInputs);

    // --- Build a map from tensor name → node that produces it
    // This is for linking sub/div node and its input nodes.
    std::unordered_map<std::string, const onnx::NodeProto *> producer_map;
    for (const auto &node : graph.node())
    {
      if (node.output_size() > 0)
        producer_map[node.output(0)] = &node;
    }

    // create an additional flatten layer.
    Layer first_layer;
    first_layer.type = LayerType::First;
    first_layer.neurons = std::vector<Neuron>(spec.numberOfInputs);
    for (const auto &variable : spec.variables)
    {
      if (variable.first.substr(0, 1) == "X")
      {
        first_layer.neurons[variable.second.id] = variable.second;
      }
    }
    first_layer.layer_size = first_layer.neurons.size();
    // initialize lower & upper biases space
    for (size_t i = 0; i < first_layer.layer_size; ++i)
    {
      first_layer.neurons[i].setLayerId(0);
      first_layer.neurons[i].setId(i);
      first_layer.biases.push_back(0.0);
      first_layer.lower_biases.push_back(0.0);
      first_layer.upper_biases.push_back(0.0);
    }
    layers.push_back(first_layer);

    // --- Iterate over nodes (layers) ---
    for (const auto &node : graph.node())
    {
      Logger::log(Logger::Level::INFO,
                  "Node: " + node.name() + "| OpType: " + node.op_type());

      Layer layer;
      if (node.op_type() == "Sub")
      {
        layer.type = LayerType::Sub;
        // continue;
      }
      else if (node.op_type() == "Div")
      {
        layer.type = LayerType::Div;
        // continue;
      }
      else if (node.op_type() == "Constant")
      {
        // NOTE: We are not adding Constant layer into layers.
        // Instead, we merge constant layer and its corresponding sub/div layer.
        // Such that, we can use sub/div layer to access its constant values.
        continue;
      }
      else if (node.op_type() == "Flatten")
      {
        layer.type = LayerType::Flatten;
      }
      else if (node.op_type() == "MatMul")
      {
        layer.type = LayerType::MatMul;
      }
      else if (node.op_type() == "Add")
      {
        layer.type = LayerType::Add;
      }
      else if (node.op_type() == "Gemm")
      {
        layer.type = LayerType::Gemm;
      }
      else if (node.op_type() == "Conv")
      {
        layer.type = LayerType::Conv;

        // Attributes
        for (const auto &attr : node.attribute())
        {
          if (attr.ints_size() > 0)
          {
            const std::string &attr_name = attr.name();
            if (attr_name == "dilations")
            {
              layer.dilation = attr.ints()[0];
            }
            else if (attr_name == "group")
            {
              layer.group = attr.ints()[0];
            }
            else if (attr_name == "kernel_shape")
            {
              // NOTE: we're assumming the kernel is a square.
              layer.kernel_height = attr.ints()[0];
              layer.kernel_width = attr.ints()[0];
            }
            else if (attr_name == "pads")
            {
              // NOTE: assume that pads are the same for all directions.
              layer.pads = attr.ints()[0];
            }
            else if (attr_name == "strides")
            {
              // NOTE: assume strides are the same for all direstions.
              layer.strides = attr.ints()[0];
            }
          }
        }
      }
      else if (node.op_type() == "Relu")
      {
        layer.type = LayerType::Relu;
      }

      // TODO: modify the rule to identify differnet types of layers.
      // Print inputs and outputs
      for (const auto &input_name : node.input())
      {
        Logger::log(Logger::Level::INFO, "Input: " + input_name);

        // If the input is a weight tensor
        if (tensor_map.find(input_name) != tensor_map.end())
        {
          // Here,
          // we consider differnet node types such as Gemm, MatMul, Add, Conv
          const auto &tensor = tensor_map[input_name];
          if (tensor.dims().size() == 1)
          {
            layer.biases = extract1DTensorData(tensor);
            layer.lower_biases = layer.biases;
            layer.upper_biases = layer.biases;
            if (layer.type != LayerType::Conv)
            {
              layer.layer_size = layer.biases.size();
            }
            Logger::log(Logger::Level::DEBUG,
                        " (bias tensor, size = " +
                            std::to_string(layer.biases.size()) + ")");
          }
          else if (tensor.dims().size() == 2)
          {
            layer.weights = extract2DTensorData(tensor);
            Logger::log(Logger::Level::DEBUG,
                        " (weight tensor, size = <" +
                            std::to_string(layer.weights.size()) + ", " +
                            std::to_string(layer.weights[0].size()) + ">)");

            // (output_dimension, input_dimension);
            layer.layer_size = layer.weights[0].size();
          }
          else if (tensor.dims().size() == 4 &&
                   layer.type == LayerType::Conv)
          {
            layer.convolution_weights = extract4DTensorData(tensor);
            // <output_dim, input_dim, kernel_height, kernel_weight>
            size_t output_dim = layer.convolution_weights.size();
            size_t input_dim = layer.convolution_weights[0].size();
            size_t kernel_height = layer.convolution_weights[0][0].size();
            size_t kernel_width = layer.convolution_weights[0][0][0].size();
            Logger::log(
                Logger::Level::DEBUG,
                " (convolution weight tensor, size = <" +
                    std::to_string(layer.convolution_weights.size()) + ", " +
                    std::to_string(layer.convolution_weights[0].size()) + ", " +
                    std::to_string(layer.convolution_weights[0][0].size()) +
                    ", " +
                    std::to_string(layer.convolution_weights[0][0][0].size()) +
                    ">)");
            // convert 4d tensor to 2d tensor
            layer.input_channels = input_dim;
            layer.output_channels = output_dim;
            layer.conv_input_height = input_height;
            layer.conv_input_width = input_width;
            layer.kernel_height = kernel_height;
            layer.kernel_width = kernel_width;
            layer.conv_output_height =
                (input_height + 2 * layer.pads - layer.kernel_height) /
                    layer.strides +
                1;
            layer.conv_output_width =
                (input_width + 2 * layer.pads - layer.kernel_width) /
                    layer.strides +
                1;
            layer.weights =
                convert4Dto2Dtensor(layer.convolution_weights, input_height,
                                    input_width, layer.strides, layer.pads);

            layer.layer_size = layer.weights.size();
            Logger::log(Logger::Level::DEBUG,
                        "The layer size is " +
                            std::to_string(layer.layer_size));

            input_height =
                std::floor((input_height + 2 * layer.pads -
                            layer.dilation * (layer.kernel_height - 1) - 1) /
                               layer.strides +
                           1);
            input_width =
                std::floor((input_width + 2 * layer.pads -
                            layer.dilation * (layer.kernel_width - 1) - 1) /
                               layer.strides +
                           1);
          }
          layer.neurons = std::vector<Neuron>(layer.layer_size);
        }
        else if (layer.type == LayerType::Sub ||
                 layer.type == LayerType::Div)
        {
          // Finding the constant in Sub/Div node
          if (producer_map.find(input_name) != producer_map.end())
          {
            const onnx::NodeProto *producer = producer_map[input_name];
            for (const auto &attr : producer->attribute())
            {
              if (attr.has_t())
              {
                // Attribute contains a tensor
                const onnx::TensorProto &tensor = attr.t();

                if (tensor.data_type() == onnx::TensorProto::FLOAT)
                {
                  if (!tensor.raw_data().empty())
                  {
                    const std::string &raw = tensor.raw_data();
                    const float *data =
                        reinterpret_cast<const float *>(raw.data());
                    size_t numel = raw.size() / sizeof(float);
                    for (size_t i = 0; i < numel; i++)
                    {
                      if (layer.type == LayerType::Sub)
                        layer.sub_values.push_back(data[i]);
                      else if (layer.type == LayerType::Div)
                        layer.div_values.push_back(data[i]);
                    }
                  }
                }
              }
              else if (attr.floats_size() > 0)
              {
                for (auto f : attr.floats())
                {
                  if (layer.type == LayerType::Sub)
                    layer.sub_values.push_back(f);
                  else if (layer.type == LayerType::Div)
                    layer.div_values.push_back(f);
                }
              }
            }
          }

          // Create neurons in the layer.
          layer.neurons = std::vector<Neuron>(spec.numberOfInputs);
          for (const auto &variable : spec.variables)
          {
            if (variable.first.substr(0, 1) == "X")
            {
              layer.neurons[variable.second.id] = variable.second;
            }
          }
          layer.layer_size = layer.neurons.size();
          // initialize lower & upper biases space
          for (size_t i = 0; i < layer.layer_size; ++i)
          {
            layer.biases.push_back(0.0);
            layer.lower_biases.push_back(0.0);
            layer.upper_biases.push_back(0.0);
          }
        }
        else if (layer.type == LayerType::Constant)
        {
          continue;
        }
        else if (layer.type == LayerType::Flatten)
        {
          // layer.neurons = std::vector<Neuron>(spec.numberOfInputs);
          // for (const auto &variable : spec.variables) {
          //   if (variable.first.substr(0, 1) == "X") {
          //     layer.neurons[variable.second.id] = variable.second;
          //   }
          // }
          // layer.layer_size = layer.neurons.size();
          // // initialize lower & upper biases space
          // for (size_t i = 0; i < layer.layer_size; ++i) {
          //   layer.biases.push_back(0.0);
          //   layer.lower_biases.push_back(0.0);
          //   layer.upper_biases.push_back(0.0);
          // }
          //
          layer.layer_size = layers[layers.size() - 1].layer_size;
          layer.neurons = std::vector<Neuron>(layer.layer_size);
        }
        else if (layer.type == LayerType::Relu)
        {
          layer.layer_size = layers[layers.size() - 1].layer_size;
          layer.neurons = std::vector<Neuron>(layer.layer_size);
        }
      }

      for (const auto &output_name : node.output())
      {
        Logger::log(Logger::Level::INFO, "  Output: " + output_name);
      }

      // update neuron index and layer index
      for (size_t i = 0; i < layer.layer_size; ++i)
      {
        layer.neurons[i].setId(i);
        layer.neurons[i].setLayerId(layers.size());
      }

      Logger::log(Logger::Level::WARN, "The layer size should be " +
                                           std::to_string(layer.layer_size));
      layers.push_back(layer);
    }

    google::protobuf::ShutdownProtobufLibrary();

    return true;
  }

  Specification parse(const std::string &input)
  {
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
    parser["Specifications"] = [this](const SV &sv)
    {
      return make_specifications(sv);
    };
    parser["Integer"] = [](const SV &sv)
    {
      return ASTNode(sv.token_to_number<int>());
    };
    parser["Float"] = [](const SV &sv)
    {
      return ASTNode(sv.token_to_number<double>());
    };
    parser["Identifier"] = [](const SV &sv)
    {
      return ASTNode(sv.token_to_string());
    };
    parser["BinaryOp"] = [this](const SV &sv)
    { return make_binary_op(sv); };
    parser["LogicOp"] = [this](const SV &sv)
    { return make_logic_op(sv); };
    parser["DeclareVar"] = [this](const SV &sv)
    {
      return make_declare_variable(sv);
    };
    parser["Bound"] = [this](const SV &sv)
    { return make_bound(sv); };
    parser["Conjunctive"] = [this](const SV &sv)
    {
      return make_conjunctive(sv);
    };
    parser["Assertion"] = [this](const SV &sv)
    { return make_assertion(sv); };
    parser.set_logger([](size_t line, size_t col, const std::string &msg,
                         const std::string &rule)
                      { Logger::log(Logger::Level::ERROR, "at line " + std::to_string(line) +
                                                              ", column " + std::to_string(col) +
                                                              ": " + msg + " in rule " + rule); });

    // std::cout << input.c_str() << std::endl;
    ASTNode ast;
    Specification spec;
    if (parser.parse(input.c_str(), ast))
    {
      Logger::log(Logger::Level::INFO, "Parsing succeeded!");
      ast.make_specifications(spec);
      ast.dump_spec_bounds(spec);
    }
    else
    {
      Logger::log(Logger::Level::WARN, "Parsing failed!");
    }

    return spec;
  }

private:
  ASTNode make_specifications(const SV &sv)
  {
    if (sv.size() == 1)
    {
      return std::any_cast<ASTNode>(sv[0]);
    }
    else
    {
      ASTNode root;
      for (size_t i = 0; i < sv.size(); ++i)
      {
        try
        {
          root.children.push_back(std::any_cast<ASTNode>(sv[i]));
        }
        catch (const std::bad_any_cast &e)
        {
          continue;
        }
      }
      return root;
    }
  }

  ASTNode make_binary_op(const SV &sv)
  {
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

  ASTNode make_logic_op(const SV &sv)
  {
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

  ASTNode make_declare_variable(const SV &sv)
  {
    ASTNode decl_node(ASTNodeType::DECLARATION, std::string("double"));
    for (size_t i = 0; i < sv.size(); ++i)
    {
      decl_node.children.push_back(std::any_cast<ASTNode>(sv[i]));
    }
    return decl_node;
  }

  ASTNode make_bound(const SV &sv)
  {
    ASTNode bound_node(ASTNodeType::BOUND);
    ASTNode lop_node = std::any_cast<ASTNode>(sv[0]);
    ASTNode left = std::any_cast<ASTNode>(sv[1]);
    ASTNode right = std::any_cast<ASTNode>(sv[2]);

    lop_node.children.push_back(left);
    lop_node.children.push_back(right);
    bound_node.children.push_back(lop_node);

    return bound_node;
  }

  ASTNode make_conjunctive(const SV &sv)
  {
    ASTNode conj_node(ASTNodeType::LOGIC_OP, LogicOp::And); // sv[0];
    for (size_t i = 1; i < sv.size(); ++i)
    {
      ASTNode bound_node = std::any_cast<ASTNode>(sv[i]);
      conj_node.children.push_back(bound_node);
    }

    return conj_node;
  }

  ASTNode make_assertion(const SV &sv)
  {
    ASTNode assert_node(ASTNodeType::ASSERTION);
    if (sv.size() == 1)
    {
      ASTNode bound_node = std::any_cast<ASTNode>(sv[0]);
      assert_node.children.push_back(bound_node);
    }
    else
    {
      // TODO: modify
      for (size_t i = 0; i < sv.size(); ++i)
      {
        ASTNode node = std::any_cast<ASTNode>(sv[i]);
        assert_node.children.push_back(node);
      }
    }

    return assert_node;
  }

  tensor1d extract1DTensorData(const onnx::TensorProto &tensor)
  {
    tensor1d data;

    // Case 1: float_data() directly stored
    if (tensor.float_data_size() > 0)
    {
      data.assign(tensor.float_data().begin(), tensor.float_data().end());
    }
    // Case 2: raw_data() binary blob
    else
    {
      std::string raw = tensor.raw_data();
      size_t elem_count = raw.size() / sizeof(float);
      data.resize(elem_count);
      std::memcpy(data.data(), raw.data(), raw.size());
    }

    return data;
  }

  std::vector<std::vector<float>>
  extract2DTensorData(const onnx::TensorProto &tensor)
  {
    tensor2d data;

    tensor1d temp_data;

    // Case 1: float_data() directly stored
    if (tensor.float_data_size() > 0)
    {
      temp_data.assign(tensor.float_data().begin(), tensor.float_data().end());
    }
    // Case 2: raw_data() binary blob
    else
    {
      std::string raw = tensor.raw_data();
      size_t elem_count = raw.size() / sizeof(float);
      temp_data.resize(elem_count);
      std::memcpy(temp_data.data(), raw.data(), raw.size());
    }

    // make 1d data vector to 2d data matrix;
    size_t num_rows = tensor.dims(0);
    size_t num_cols = tensor.dims(1);
    data.resize(num_rows, tensor1d(num_cols));
    for (size_t r = 0; r < num_rows; ++r)
    {
      for (size_t c = 0; c < num_cols; ++c)
      {
        data[r][c] = temp_data[r * num_cols + c];
      }
    }

    return data;
  }

  tensor4d extract4DTensorData(const onnx::TensorProto &tensor)
  {

    tensor1d flat_data;

    // Case 1: float_data() directly stored
    if (tensor.float_data_size() > 0)
    {
      flat_data.assign(tensor.float_data().begin(), tensor.float_data().end());
    }
    // Case 2: raw_data() binary blob
    else
    {
      std::string raw = tensor.raw_data();
      size_t elem_count = raw.size() / sizeof(float);
      flat_data.resize(elem_count);
      std::memcpy(flat_data.data(), raw.data(), raw.size());
    }

    // Read dims
    if (tensor.dims_size() != 4)
      throw std::runtime_error("Tensor is not 4D");

    size_t C_out = tensor.dims(0);
    size_t C_in = tensor.dims(1);
    size_t kH = tensor.dims(2);
    size_t kW = tensor.dims(3);

    // Allocate 4D vector
    tensor4d data(C_out, tensor3d(C_in, tensor2d(kH, tensor1d(kW))));

    // Fill data
    size_t idx = 0;
    for (size_t oc = 0; oc < C_out; ++oc)
    {
      for (size_t ic = 0; ic < C_in; ++ic)
      {
        for (size_t h = 0; h < kH; ++h)
        {
          for (size_t w = 0; w < kW; ++w)
          {
            data[oc][ic][h][w] = flat_data[idx++];
          }
        }
      }
    }

    return data;
  }

  tensor2d convert4Dto2Dtensor(const tensor4d &weights, size_t input_H,
                               size_t input_W, size_t stride, size_t pad)
  {
    const size_t output_channels = weights.size();
    const size_t input_channels = weights[0].size();
    const size_t kernel_H = weights[0][0].size();
    const size_t kernel_W = weights[0][0][0].size();

    const size_t output_H = (input_H + 2 * pad - kernel_H) / stride + 1;
    const size_t output_W = (input_W + 2 * pad - kernel_W) / stride + 1;

    const size_t input_size = input_channels * input_H * input_W;
    const size_t output_size = output_channels * output_H * output_W;
    Logger::log(Logger::Level::DEBUG,
                "The output size is " + std::to_string(output_size));

    tensor2d result_weights(output_size, tensor1d(input_size, 0.0f));
    // FIX: The implementation seems not correct.
    // size_t row = 0;
    // for (size_t oc = 0; oc < output_channels; ++oc) {
    //   for (size_t oh = 0; oh < output_H; ++oh) {
    //     for (size_t ow = 0; ow < output_W; ++ow) {
    //
    //       // bounds check for row (defensive)
    //       if (row >= output_size) {
    //         std::cerr << "ERROR: row >= output_size: row=" << row
    //                   << " output_size=" << output_size << std::endl;
    //         assert(false);
    //       }
    //
    //       for (size_t ic = 0; ic < input_channels; ++ic) {
    //         for (size_t kh = 0; kh < kernel_H; ++kh) {
    //           for (size_t kw = 0; kw < kernel_W; ++kw) {
    //             // use signed integers for intermediate coordinates
    //             int ih = static_cast<int>(oh * stride) + static_cast<int>(kh)
    //             -
    //                      static_cast<int>(pad);
    //             int iw = static_cast<int>(ow * stride) + static_cast<int>(kw)
    //             -
    //                      static_cast<int>(pad);
    //
    //             // only write when ih/iw inside input bounds
    //             if (ih >= 0 && ih < static_cast<int>(input_H) && iw >= 0 &&
    //                 iw < static_cast<int>(input_W)) {
    //
    //               size_t col = ic * (input_H * input_W) +
    //                            static_cast<size_t>(ih) * input_W +
    //                            static_cast<size_t>(iw);
    //
    //               // defensive check for col bounds
    //               if (col >= input_size) {
    //                 std::cerr << "ERROR: computed col out of range: col=" <<
    //                 col
    //                           << " input_size=" << input_size
    //                           << " (oc,ic,oh,ow,kh,kw)=(" << oc << "," << ic
    //                           << "," << oh << "," << ow << "," << kh << ","
    //                           << kw << ")\n";
    //                 assert(false);
    //               }
    //
    //               // write weight value
    //               result_weights[row][col] = weights[oc][ic][kh][kw];
    //             }
    //           }
    //         }
    //       }
    //       ++row;
    //     }
    //   }
    // }

    // TEST: marabou way
    for (size_t i = 0; i < output_W; ++i)
    {
      for (size_t j = 0; j < output_H; ++j)
      {
        for (size_t k = 0; k < output_channels; ++k)
        {
          size_t row = k * (output_H * output_W) + j * output_W + i;

          for (size_t di = 0; di < kernel_W; ++di)
          {
            for (size_t dj = 0; dj < kernel_H; ++dj)
            {
              for (size_t dk = 0; dk < input_channels; ++dk)
              {
                int wIndex = static_cast<int>(stride * i + di - pad);
                int hIndex = static_cast<int>(stride * j + dj - pad);

                if (wIndex >= 0 && wIndex < (int)input_W && hIndex >= 0 &&
                    hIndex < (int)input_H)
                {
                  size_t col =
                      dk * (input_H * input_W) + hIndex * input_W + wIndex;
                  result_weights[row][col] =
                      weights[k][dk][dj][di]; // same index pattern as Marabou
                }
              }
            }
          }
        }
      }
    }

    return result_weights;
  }
};

#endif // NEURAL_NETWORK_PARSER_HPP
