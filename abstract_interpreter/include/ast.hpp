// copyright 2024 Yi-Nung Tsao

#ifndef ABSTRACT_INTERPRETER_AST_HPP
#define ABSTRACT_INTERPRETER_AST_HPP

#include <variant>
#include <cmath>

enum class BinOp {ADD, SUB, MUL, DIV};
std::ostream& operator<<(std::ostream& os, BinOp op) {
    switch (op) {
        case BinOp::ADD: os << "+"; break;
        case BinOp::SUB: os << "-"; break;
        case BinOp::MUL: os << "*"; break;
        case BinOp::DIV: os << "/"; break;
    }
    return os;
}

enum class LogicOp {LE, LEQ, GE, GEQ, EQ, NEQ};
std::ostream& operator<<(std::ostream& os, LogicOp lop){
    switch (lop){
        case LogicOp::LE: os << "<"; break;
        case LogicOp::LEQ: os << "<="; break;
        case LogicOp::GE: os << ">"; break;
        case LogicOp::GEQ: os << ">="; break;
        case LogicOp::EQ: os << "=="; break;
        case LogicOp::NEQ: os << "!="; break;
    }
    return os;
}

enum class NodeType {VARIABLE, INTEGER, PRE_CON, POST_CON, ARITHM_OP, LOGIC_OP, DECLARATION, ASSIGNMENT, IFELSE, WHILELOOP, SEQUENCE};
std::ostream& operator<<(std::ostream& os, NodeType type) {
    switch (type) {
        case NodeType::VARIABLE: os << "Variable"; break;
        case NodeType::INTEGER: os << "Integer"; break;
        case NodeType::PRE_CON: os << "Pre conditions"; break;
        case NodeType::POST_CON: os << "Post conditions"; break;
        case NodeType::ARITHM_OP: os << "Arithmetic Operation"; break;
        case NodeType::LOGIC_OP: os << "Logic Operation"; break;
        case NodeType::DECLARATION: os << "Declaration"; break;
        case NodeType::ASSIGNMENT: os << "Assignment"; break;
        case NodeType::IFELSE: os << "If-Else"; break;
        case NodeType::WHILELOOP: os << "While-Loop"; break;
        case NodeType::SEQUENCE: os << "Sequence"; break;
    }
    return os;
}

struct ASTNode {
    using VType = std::variant<std::string, int, BinOp, LogicOp>;
    using ASTNodes = std::vector<ASTNode>;

    NodeType type;
    VType value;
    ASTNodes children;

    ASTNode(): type(NodeType::INTEGER), value(0) {}
    ASTNode(const std::string& name): type(NodeType::VARIABLE), value(name){}
    ASTNode(const int num): type(NodeType::INTEGER), value(num) {}
    ASTNode(BinOp bop, ASTNode left, ASTNode right)
        : type(NodeType::ARITHM_OP), value(bop){
            children.push_back(left);
            children.push_back(right);
        }
    ASTNode(LogicOp lop, ASTNode left, ASTNode right)
        : type(NodeType::LOGIC_OP), value(lop){
            children.push_back(left);
            children.push_back(right);
        }
    ASTNode(NodeType t): type(t){}
    ASTNode(NodeType t, const std::string& name): type(t), value(name){}
    ASTNode(NodeType t, const VType& value): type(t), value(value) {}

    static void printVariant(const std::variant<std::string, int, BinOp, LogicOp>& value) {
        std::visit([](const auto& v) {
            std::cout << v << std::endl;
        }, value);
    }

    void print(int depth = 0) const {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "NodeType: " << type << ", Value: ";
        printVariant(value);
        for (const auto& child : children) {
            child.print(depth + 1);
        }
    }
};

#endif
