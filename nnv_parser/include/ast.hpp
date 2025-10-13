#ifndef NEURAL_NETWORK_AST_HPP
#define NEURAL_NETWORK_AST_HPP

#include <assert.h>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

#include "network_structure.hpp"

enum class BinaryOp { LessEqual, GreaterEqual };
enum class LogicOp { And, Or };

enum class ASTNodeType {
  VARIABLE,
  INTEGER,
  DOUBLE,
  LOGIC_OP,
  DECLARATION,
  ASSERTION,
  BOUND,
  BINARY_OP
};

struct ASTNode {
  using VType = std::variant<std::string, int, double, BinaryOp, LogicOp>;
  using ASTNodes = std::vector<ASTNode>;

  ASTNodeType type;
  VType value;
  ASTNodes children;

  ASTNode() : type(ASTNodeType::INTEGER), value(0) {}
  ASTNode(const std::string &name) : type(ASTNodeType::VARIABLE), value(name) {}
  ASTNode(const int num) : type(ASTNodeType::INTEGER), value(num) {}
  ASTNode(const double num) : type(ASTNodeType::DOUBLE), value(num) {}
  ASTNode(BinaryOp lop, ASTNode left, ASTNode right)
      : type(ASTNodeType::LOGIC_OP), value(lop) {
    children.push_back(left);
    children.push_back(right);
  }
  ASTNode(ASTNodeType t) : type(t) {}
  ASTNode(ASTNodeType t, const std::string &name) : type(t), value(name) {}
  ASTNode(ASTNodeType t, const VType &value) : type(t), value(value) {}

  void make_specifications(Specification &spec) {

    if (type == ASTNodeType::DECLARATION) {
      for (size_t i = 0; i < children.size(); ++i) {
        ASTNode node = children[i];
        std::string name = std::get<std::string>(node.value);
        spec.variables[name] = Neuron();

        if (name.substr(0, 1) == "X") {
          spec.variables[name].setId(spec.numberOfInputs);
          spec.numberOfInputs++;
        } else {
          spec.variables[name].setId(spec.numberOfOutputs);
          spec.numberOfOutputs++;
        }
      }
    }
    if (type == ASTNodeType::ASSERTION) {
      ASTNode child = children[0];
      if (child.type == ASTNodeType::BOUND) {
        ASTNode bop_node = child.children[0];
        BinaryOp bop = std::get<BinaryOp>(bop_node.value);

        ASTNode variable = bop_node.children[0];
        std::string var_name = std::get<std::string>(variable.value);

        ASTNode bound = bop_node.children[1];
        if (bound.type == ASTNodeType::DOUBLE) {
          // In the input layer.
          double bound_value = std::get<double>(bound.value);

          if (bop == BinaryOp::LessEqual)
            spec.variables[var_name].bounds.setUb(bound_value);
          else if (bop == BinaryOp::GreaterEqual)
            spec.variables[var_name].bounds.setLb(bound_value);
        } else if (bound.type == ASTNodeType::VARIABLE) {
          // In the output layer.
          std::string var2_name = std::get<std::string>(bound.value);
          std::vector<int> postcondition(spec.numberOfOutputs, 0);
          if (bop == BinaryOp::LessEqual) {
            postcondition[spec.variables[var_name].id] = 1;
            postcondition[spec.variables[var2_name].id] = 2;
          } else if (bop == BinaryOp::GreaterEqual) {
            postcondition[spec.variables[var_name].id] = 1;
            postcondition[spec.variables[var2_name].id] = 2;
          }
        }
      } else if (child.type == ASTNodeType::LOGIC_OP) {
        LogicOp lop = std::get<LogicOp>(child.value);
        assert(lop == LogicOp::Or);

        for (size_t i = 1; i < children.size(); ++i) {
          std::vector<int> postcondition(spec.numberOfOutputs, 0);
          double bias = 0;

          ASTNode conj_node = children[i];
          ASTNode bound_node = conj_node.children[0];
          ASTNode bop_node = bound_node.children[0];
          BinaryOp bop = std::get<BinaryOp>(bop_node.value);
          ASTNode variable1 = bop_node.children[0];
          ASTNode variable2 = bop_node.children[1];

          if (bop == BinaryOp::LessEqual) {
            std::string var1_name = std::get<std::string>(variable1.value);
            std::string var2_name = std::get<std::string>(variable2.value);
            postcondition[spec.variables[var1_name].id] = 1;
            postcondition[spec.variables[var2_name].id] = -1;
          } else if (bop == BinaryOp::GreaterEqual) {
            std::string var1_name = std::get<std::string>(variable1.value);
            std::string var2_name = std::get<std::string>(variable2.value);
            postcondition[spec.variables[var1_name].id] = -1;
            postcondition[spec.variables[var2_name].id] = 1;
          }

          spec.A.push_back(postcondition);
          spec.b.push_back(bias);
        }
      }
    }

    for (size_t i = 0; i < children.size(); ++i) {
      children[i].make_specifications(spec);
    }

    return;
  }

  void print_bounds(Specification &spec) {
    std::cout << "-----------------------------------------------------------"
              << std::endl;
    std::cout << "number of inputs = " << spec.numberOfInputs << std::endl;
    std::cout << "number of outputs = " << spec.numberOfOutputs << std::endl;
    std::cout << "-----------------------------------------------------------"
              << std::endl;
    for (auto variable = spec.variables.begin();
         variable != spec.variables.end(); variable++) {
      std::cout << variable->first << " (id = " << variable->second.id << " )"
                << " = " << "[ " << variable->second.bounds.getLb() << ", "
                << variable->second.bounds.getUb() << " ]" << std::endl;
    }
    std::cout << "-----------------------------------------------------------"
              << std::endl;
    std::cout << std::endl;
    std::cout << "Postconditions matrix and bias: " << std::endl;
    std::cout << "A matrix: " << std::endl;
    for (auto i : spec.A) {
      for (auto j : i) {
        std::cout << j << ", ";
      }
      std::cout << std::endl;
    }
    std::cout << "b vector: " << std::endl;
    for (auto i : spec.b) {
      std::cout << i << std::endl;
    }
    std::cout << "-----------------------------------------------------------"
              << std::endl;
    return;
  }
};

#endif // NEURAL_NETWORK_AST_HPP
