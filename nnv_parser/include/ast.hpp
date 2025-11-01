#ifndef NEURAL_NETWORK_AST_HPP
#define NEURAL_NETWORK_AST_HPP

#include <assert.h>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

#include "log.hpp"
#include "specifications.hpp"

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
          spec.variables[name].bounds = Interval();
          spec.numberOfInputs++;
        } else {
          spec.variables[name].setId(spec.numberOfOutputs);
          spec.variables[name].bounds = Interval();
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
          char relation;
          double bias = 0;

          if (bop == BinaryOp::LessEqual) {
            postcondition[spec.variables[var_name].id] = 1;
            postcondition[spec.variables[var2_name].id] = -1;

            relation = 'L';
          } else if (bop == BinaryOp::GreaterEqual) {
            postcondition[spec.variables[var_name].id] = -1;
            postcondition[spec.variables[var2_name].id] = 1;

            relation = 'G';
          }

          if (spec.A.empty() && spec.b.empty()) {
            spec.A.push_back(std::vector<std::vector<int>>());
            spec.b.push_back(std::vector<double>());

            spec.relations.push_back(std::vector<char>());
          }
          spec.A[0].push_back(postcondition);
          spec.b[0].push_back(bias);

          spec.relations[0].push_back(relation);
        }
      } else if (child.type == ASTNodeType::LOGIC_OP) {
        LogicOp lop = std::get<LogicOp>(child.value);
        assert(lop == LogicOp::Or);

        for (size_t i = 1; i < children.size(); ++i) {
          ASTNode conj_node = children[i];
          std::vector<std::vector<int>> postconditions;
          std::vector<double> biases;
          std::vector<char> relation;

          for (size_t j = 0; j < conj_node.children.size(); ++j) {
            std::vector<int> postcondition(spec.numberOfOutputs, 0);
            double bias = 0;

            ASTNode bound_node = conj_node.children[j];
            ASTNode bop_node = bound_node.children[0];
            BinaryOp bop = std::get<BinaryOp>(bop_node.value);
            ASTNode variable = bop_node.children[0];
            std::string var_name = std::get<std::string>(variable.value);
            ASTNode bound = bop_node.children[1];
            if (bound.type == ASTNodeType::DOUBLE) {
              // TODO: temporary skip this part. We don't consider disjunctive
              // in preconditions. In the input layer.
              // This is only for prop_6.vnnlib
              double bound_value = std::get<double>(bound.value);
              std::cout << "******************************\n";
              std::cout << bound_value << "\n";
              std::cout << "******************************\n";
              // if (spec.variables[var_name].bounds.size() == i - 1) {
              //   spec.variables[var_name].bounds.push_back(Interval());
              // }
              // if (bop == BinaryOp::LessEqual) {
              //   spec.variables[var_name].bounds[i - 1].setUb(bound_value);
              // } else if (bop == BinaryOp::GreaterEqual) {
              //   spec.variables[var_name].bounds[i - 1].setLb(bound_value);
              // }
            } else if (bound.type == ASTNodeType::VARIABLE) {
              // In the output layer.
              if (bop == BinaryOp::LessEqual) {
                std::string var_name = std::get<std::string>(variable.value);
                std::string var2_name = std::get<std::string>(bound.value);
                postcondition[spec.variables[var_name].id] = 1;
                postcondition[spec.variables[var2_name].id] = -1;

                relation.push_back('L');
              } else if (bop == BinaryOp::GreaterEqual) {
                std::string var1_name = std::get<std::string>(variable.value);
                std::string var2_name = std::get<std::string>(bound.value);
                postcondition[spec.variables[var1_name].id] = -1;
                postcondition[spec.variables[var2_name].id] = 1;

                relation.push_back('G');
              }

              postconditions.push_back(postcondition);
              biases.push_back(bias);
            }
          }
          spec.A.push_back(postconditions);
          spec.b.push_back(biases);

          spec.relations.push_back(relation);
        }
      }
    }

    for (size_t i = 0; i < children.size(); ++i) {
      children[i].make_specifications(spec);
    }

    return;
  }

  void dump_spec_bounds(Specification &spec) const {
    Logger::log(Logger::Level::INFO,
                "number of inputs = " + std::to_string(spec.numberOfInputs));
    Logger::log(Logger::Level::INFO,
                "number of outputs = " + std::to_string(spec.numberOfOutputs));

    Logger::log(Logger::Level::INFO, "Postconditions matrix and bias: ");
    Logger::log(Logger::Level::INFO, "A matrix: ");
    for (auto i : spec.A) {
      for (auto j : i) {
        for (auto k : j)
          std::cout << k << ", ";
        std::cout << std::endl;
      }
      std::cout << std::endl;
    }

    Logger::log(Logger::Level::INFO, "b vector: ");
    for (auto i : spec.b) {
      for (auto j : i)
        std::cout << j << std::endl;
      std::cout << std::endl;
    }

    Logger::log(Logger::Level::INFO, "relation of each postcondition: ");
    for (auto i : spec.relations) {
      for (auto j : i)
        std::cout << j;
      std::cout << std::endl;
    }

    return;
  }
};

#endif // NEURAL_NETWORK_AST_HPP
