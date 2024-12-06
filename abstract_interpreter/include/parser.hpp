// copyright 2024 Yi-Nung Tsao

#ifndef ABSTRACT_INTERPRETER_PARSER_HPP
#define ABSTRACT_INTERPRETER_PARSER_HPP


#include "peglib.h"
#include <assert.h>
#include <iostream>

#include "ast.hpp"

class AbstractInterpreterParser{
    using SV = peg::SemanticValues;
    
public:
    // ASTNode root;

    ASTNode parse(const std::string& input){
        peg::parser parser(R"(
            Program     <- Statements*
            Statements  <- DeclareVar / Assignment / Increment / IfElse / WhileLoop / Block / PreCon / PostCon / Comment
            Integer     <- < [+-]? [0-9]+ >
            Identifier  <- < [a-zA-Z_][a-zA-Z0-9_]* >
            SeqOp       <- '+' / '-'
            PreOp       <- '*' / '/'
            LogicOp     <- '<=' / '>=' / '==' / '!=' / '<' / '>'
            DeclareVar  <- 'int' Identifier ('=' Integer / ',' Identifier)* ';'
            PreCon      <- '/*!npk' Identifier 'between' Integer 'and' Integer '*/'
            PostCon     <- 'assert' '(' Expression ')' ';'
            Assignment  <- Identifier '=' Expression ';'
            Increment   <- Identifier '++' ';'
            Block       <- ('void main' '(' ')')? '{' Statements* '}'
            IfElse      <- 'if' '(' Expression ')' (Block / Statements) ('else' (Block / Statements))?
            WhileLoop   <- 'while' '(' Expression ')' (Block / Statements)

            Expression  <- Term ((SeqOp / LogicOp) Term)*
            Term        <- Factor (PreOp Factor)*
            Factor      <- '-' Factor / Integer / Identifier / '(' Expression ')'

            ~Comment    <- '//' [^\n\r]* [ \n\r\t]*
            %whitespace <- [ \n\r\t]*
        )");
        assert(static_cast<bool>(parser) == true);

        // // setup actions
        parser["Program"] = [this](const SV& sv){return make_program(sv);};
        parser["Integer"] = [](const SV& sv){return ASTNode(sv.token_to_number<int>());};
        parser["Identifier"] = [](const SV& sv){return ASTNode(sv.token_to_string());};
        parser["SeqOp"] = [this](const SV& sv){return make_seq_op(sv);};
        parser["PreOp"] = [this](const SV& sv){return make_pre_op(sv);};
        parser["LogicOp"] = [this](const SV& sv){return make_logic_op(sv);};
        parser["DeclareVar"] = [this](const SV& sv){return make_decl_var(sv);};
        parser["PreCon"] = [this](const SV& sv){return make_pre_con(sv);};
        parser["PostCon"] = [this](const SV& sv){return make_post_con(sv);};
        parser["Assignment"] = [this](const SV& sv){return make_assign(sv);};
        parser["Increment"] = [this](const SV& sv){return make_increment(sv);};
        parser["Block"] = [this](const SV& sv){return make_block(sv);};
        parser["IfElse"] = [this](const SV& sv){return make_ifelse(sv);};
        parser["WhileLoop"] = [this](const SV& sv){return make_whileloop(sv);};
        parser["Expression"] = [this](const SV& sv){return make_expr(sv);};
        parser["Term"] = [this](const SV& sv){return make_term(sv);};
        parser["Factor"] = [this](const SV& sv){return make_factor(sv);};
        parser.set_logger([](size_t line, size_t col, const std::string& msg, const std::string &rule) {
            std::cerr << line << ":" << col << ": " << msg << "\n";
        });

        ASTNode root;
        if (parser.parse(input.c_str(), root)){
            std::cout << "Parsing succeeded!" << std::endl;
        }else{
            std::cerr << "Parsing failed!" << std::endl;
        }   
        return root;
    }

private:
    ASTNode make_program(const SV& sv){
        if (sv.size() == 1){
            return std::any_cast<ASTNode>(sv[0]);
        }
        else{
            ASTNode root;
            for (size_t i = 0; i < sv.size(); ++i){
                try{
                    root.children.push_back(std::any_cast<ASTNode>(sv[i]));
                }
                catch(std::bad_any_cast){
                    // for dealing with the comments in the program.
                    // because I didn't cast the comments as ASTNode, it should be "string".
                    // then here, I just skip it if there is any comment.
                    continue;
                }
            }
            return root;
        }
    }

    ASTNode make_decl_var(const SV& sv){
        ASTNode decl_node(NodeType::DECLARATION, std::string("int"));
        for (size_t i = 0; i < sv.size(); ++i){
            decl_node.children.push_back(std::any_cast<ASTNode>(sv[i]));
        }
        return decl_node;
    }

    ASTNode make_pre_con(const SV& sv){
        ASTNode pre_con_node(NodeType::PRE_CON, std::string("PreCon"));
        ASTNode var(std::any_cast<ASTNode>(sv[0]));

        // LB
        ASTNode lb(NodeType::LOGIC_OP, std::string("<="));
        lb.children.push_back(std::any_cast<ASTNode>(sv[1]));
        lb.children.push_back(var);

        // UB 
        ASTNode ub(NodeType::LOGIC_OP, std::string(">="));
        ub.children.push_back(std::any_cast<ASTNode>(sv[2]));
        ub.children.push_back(var);

        pre_con_node.children.push_back(lb);
        pre_con_node.children.push_back(ub);
        return pre_con_node;
    }

    ASTNode make_post_con(const SV& sv){
        ASTNode post_con_node(NodeType::POST_CON, std::string("PostCon"));
        ASTNode expr(std::any_cast<ASTNode>(sv[0]));
        post_con_node.children.push_back(expr);
        return post_con_node;
    }

    ASTNode make_seq_op(const SV& sv){
        ASTNode op_node(NodeType::ARITHM_OP);
        std::string op = sv.token_to_string();
        if (op == "+") op_node.value = BinOp::ADD;
        else if (op == "-") op_node.value = BinOp::SUB;
        return op_node;
    }

    ASTNode make_pre_op(const SV& sv){
        ASTNode op_node(NodeType::ARITHM_OP);
        std::string op = sv.token_to_string();
        if (op == "*") op_node.value = BinOp::MUL;
        else if (op == "/") op_node.value = BinOp::DIV;
        return op_node;
    }

    ASTNode make_logic_op(const SV& sv){
        ASTNode lop_node(NodeType::LOGIC_OP);
        std::string lop = sv.token_to_string();
        if (lop == "<") lop_node.value = LogicOp::LE;
        else if (lop == ">") lop_node.value = LogicOp::GE;
        else if (lop == "<=") lop_node.value = LogicOp::LEQ;
        else if (lop == ">=") lop_node.value = LogicOp::GEQ;
        else if (lop == "==") lop_node.value = LogicOp::EQ;
        else if (lop == "!=") lop_node.value = LogicOp::NEQ;
        return lop_node;
    }

    ASTNode make_expr(const SV& sv){
        if (sv.size() == 1){
            return std::any_cast<ASTNode>(sv[0]);
        }
        else if (sv.size() == 3){
            ASTNode op = std::any_cast<ASTNode>(sv[1]);
            ASTNode expr(op.type, op.value);
            expr.children.push_back(std::any_cast<ASTNode>(sv[0]));
            expr.children.push_back(std::any_cast<ASTNode>(sv[2]));
            return expr;
        }
        else{
            ASTNode expr(NodeType::ARITHM_OP, std::any_cast<ASTNode>(sv[1]).value);
            expr.children.push_back(std::any_cast<ASTNode>(sv[0]));
            ASTNode op;
            size_t i = 3;
            for (i; i < sv.size(); i+=2){
                op = std::any_cast<ASTNode>(sv[i]);
                op.children.push_back(std::any_cast<ASTNode>(sv[i-1]));
                if (i+2 < sv.size()) 
                    expr.children.push_back(op);
            }
            op.children.push_back(std::any_cast<ASTNode>(sv[i-1]));
            expr.children.push_back(op);
            return expr;
        }
    }

    ASTNode make_term(const SV& sv){
        if (sv.size() == 1){
            return std::any_cast<ASTNode>(sv[0]);
        }
        else if (sv.size() == 3){
            ASTNode term(NodeType::ARITHM_OP, std::any_cast<ASTNode>(sv[1]).value);
            term.children.push_back(std::any_cast<ASTNode>(sv[0]));
            term.children.push_back(std::any_cast<ASTNode>(sv[2]));
            return term;
        }
        else{
            ASTNode term(NodeType::ARITHM_OP, std::any_cast<ASTNode>(sv[1]).value);
            term.children.push_back(std::any_cast<ASTNode>(sv[0]));
            size_t i = 3;
            for (i; i < sv.size(); i+=2){
                ASTNode op(NodeType::ARITHM_OP, std::any_cast<ASTNode>(sv[i]).value);
                term.children.push_back(op);
                term.children.push_back(std::any_cast<ASTNode>(sv[i-1]));
            }
            term.children.push_back(std::any_cast<ASTNode>(sv[i-1]));
            return term;
        }
    }

    ASTNode make_factor(const SV& sv){
        if (sv.choice() == 0){
            // for the case: x = -y; 
            // we're going to transform it into x = 0 - y;
            ASTNode sign(NodeType::ARITHM_OP, std::string("-"));
            sign.children.push_back(ASTNode(0));
            sign.children.push_back(std::any_cast<ASTNode>(sv[0]));
            return sign;
        }
        else{
            return std::any_cast<ASTNode>(sv[0]);
        }
    }

    ASTNode make_assign(const SV& sv){
        ASTNode assign_node(NodeType::ASSIGNMENT, std::string("="));
        ASTNode var = std::any_cast<ASTNode>(sv[0]);
        ASTNode expr = std::any_cast<ASTNode>(sv[1]);
        assign_node.children.push_back(var);
        assign_node.children.push_back(expr);
        return assign_node;
    }
    
    ASTNode make_increment(const SV& sv){
        ASTNode increment_node(NodeType::ASSIGNMENT, std::string("="));
        
        ASTNode var = std::any_cast<ASTNode>(sv[0]);
        ASTNode plus_op(NodeType::ARITHM_OP, std::string("+"));
        plus_op.children.push_back(var);
        plus_op.children.push_back(ASTNode(1));

        increment_node.children.push_back(var);
        increment_node.children.push_back(plus_op);

        return increment_node;
    }

    ASTNode make_block(const SV& sv){
        if (sv.size() == 1){
            return std::any_cast<ASTNode>(sv[0]);
        }
        else{
            ASTNode seq(NodeType::SEQUENCE, std::string(";"));
            for (size_t i = 0; i < sv.size(); ++i){
                try{
                    seq.children.push_back(std::any_cast<ASTNode>(sv[i]));
                }
                catch(std::bad_any_cast){
                    // pre-condition in this version is comment, still is string;
                    // so, we cannot cast it as ASTNode;
                    continue;
                }
            }
            return seq;
        }
    }

    ASTNode make_ifelse(const SV& sv){
        ASTNode ifelse_node(NodeType::IFELSE, std::string("IfElse"));
        for (size_t i = 0; i < sv.size(); ++i){
            ASTNode mid_node;
            if (i == 0) mid_node = ASTNode(NodeType::IFELSE, std::string("Condition")); 
            else if (i == 1) mid_node = ASTNode(NodeType::IFELSE, std::string("If-Body"));
            else if (i == 2) mid_node = ASTNode(NodeType::IFELSE, std::string("Else-Body"));
            ASTNode node = std::any_cast<ASTNode>(sv[i]);
            mid_node.children.push_back(node);
            ifelse_node.children.push_back(mid_node);
        }
        return ifelse_node;
    }

    ASTNode make_whileloop(const SV& sv){
        ASTNode whileloop_node(NodeType::WHILELOOP, std::string("WhileLoop"));
        for (size_t i = 0; i < sv.size(); ++i){
            ASTNode mid_node;
            if (i == 0) mid_node = ASTNode(NodeType::WHILELOOP, std::string("Condition"));
            else if (i == 1) mid_node = ASTNode(NodeType::WHILELOOP, std::string("While-Body"));
            ASTNode node = std::any_cast<ASTNode>(sv[i]);
            mid_node.children.push_back(node);
            whileloop_node.children.push_back(mid_node);
        }
        return whileloop_node;
    }
};

#endif
