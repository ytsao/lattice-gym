#ifndef NEURAL_NETWORK_AST_HPP
#define NEURAL_NETWORK_AST_HPP

#include <cmath>
#include <string>
#include <variant>
#include <vector>

#include "network_structure.hpp"

enum class LogicOp { LessEqual, GreaterEqual };

enum class ASTNodeType {
  VARIABLE,
  INTEGER,
  DOUBLE,
  LOGIC_OP,
  DECLARATION,
  ASSERTION
};

struct ASTNode {
  using VType = std::variant<std::string, int, double, LogicOp>;
  using ASTNodes = std::vector<ASTNode>;

  ASTNodeType type;
  VType value;
  ASTNodes children;

  ASTNode() : type(ASTNodeType::INTEGER), value(0) {}
  ASTNode(const std::string &name) : type(ASTNodeType::VARIABLE), value(name) {}
  ASTNode(const int num) : type(ASTNodeType::INTEGER), value(num) {}
  ASTNode(const double num) : type(ASTNodeType::DOUBLE), value(num) {}
  ASTNode(LogicOp lop, ASTNode left, ASTNode right)
      : type(ASTNodeType::LOGIC_OP), value(lop) {
    children.push_back(left);
    children.push_back(right);
  }
  ASTNode(ASTNodeType t) : type(t) {}
  ASTNode(ASTNodeType t, const std::string &name) : type(t), value(name) {}
  ASTNode(ASTNodeType t, const VType &value) : type(t), value(value) {}

  void make_specifications(Specification &spec) {

    if (type == ASTNodeType::DECLARATION) {
      std::string name = std::get<std::string>(value);
      spec.variables[std::get<std::string>(value)] = Interval();

      if (name.substr(0, 1) == name)
        spec.numberOfInputs++;
      else
        spec.numberOfOutputs++;
    }
    if (type == ASTNodeType::ASSERTION) {
      ASTNode lop_node = children[0];
      LogicOp lop = std::get<LogicOp>(lop_node.value);

      ASTNode variable = lop_node.children[0];
      std::string var_name = std::get<std::string>(variable.value);

      ASTNode bound = lop_node.children[1];
      double bound_value = std::get<double>(bound.value);

      if (lop == LogicOp::LessEqual)
        spec.variables[var_name].setUb(bound_value);
      else if (lop == LogicOp::GreaterEqual)
        spec.variables[var_name].setLb(bound_value);
    }

    for (size_t i = 0; i < children.size(); ++i) {
      children[i].make_specifications(spec);
    }

    return;
  }
};

#endif // NEURAL_NETWORK_AST_HPP
