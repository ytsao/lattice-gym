// using peglib to parse vnnlib file;

#ifndef NEURAL_NETWORK_PARSER_HPP
#define NEURAL_NETWORK_PARSER_HPP

#include "peglib.h"
#include <assert.h>
#include <iostream>
#include <stdexcept>

#include "ast.hpp"
#include "network_structure.hpp"

class NeuralNetworkParser {
  using SV = peg::SemanticValues;

public:
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
                            Conjunctive             <- '(' LogicOp [ \t]* Bound ')' 
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
    ASTNode bound_node = std::any_cast<ASTNode>(sv[1]);

    conj_node.children.push_back(bound_node);

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
};

#endif // NEURAL_NETWORK_PARSER_HPP
