// copyright 2024 Yi-Nung Tsao

#ifndef ABSTRACT_INTERPRETER_PARSER_HPP
#define ABSTRACT_INTERPRETER_PARSER_HPP


#include "peglib.h"
#include <assert.h>
#include <iostream>

#include "ast.hpp"

class AbstractInterpreterParser{
    using SV = peg::SemanticValues;
    std::map<std::string, int> var_arrays; //TODO: modify the concrete value into interval.

public:
    void parse(const std::string& input){
        peg::parser parser(R"(
            Statements  <- ( DeclareVar / Assignment )+
            Integer     <- < [+-]? [0-9]+ >
            TermOp      <- < [+-] >
            FactOp      <- < [*/] >
            #ExpoOp      <- < '^' >
            Identifier  <- < [a-zA-Z_][a-zA-Z0-9_]* >
            DeclareVar   <- 'int' Identifier ('=' Integer / ',' Identifier )* ';'

            Factor      <-  Identifier / Integer / '(' Expression ')'
            Term        <-  Factor (FactOp Factor)*
            Expression  <-  Term (TermOp Term)*
            Assignment  <-  Identifier '=' Expression ';'
            
            #Factor      <- Integer / Identifier / '(' Expression ')'
            #Exponent    <- Factor (ExpoOp Factor)*
            #Term        <- Exponent (FactOp Exponent)*
            #Expression  <- Term (TermOp Term)*

            #LogicOp     <- '==' / '!=' / '<' / '<=' / '>' / '>='
            #Condition   <- (Identifier / Integer) LogicOp (Identifier / Integer)

            ~Comment    <- '//' [^\n\r]* [ \n\r\t]*
            %whitespace <- [ \n\r\t]*
        )");
        assert(static_cast<bool>(parser) == true);

        // setup actions
        parser["Integer"] = [](const SV& sv){return sv.token_to_number<int>();};      
        parser["TermOp"] = [](const SV& sv){return sv.token_to_string();};
        parser["FactOp"] = [](const SV& sv){return sv.token_to_string();};
        parser["Identifier"] = [](const SV& sv){return sv.token_to_string();};
        parser["Factor"] = [](const SV& sv){return sv.token_to_string();};
        parser["Term"] = [this](const SV& sv){return make_term(sv);};
        parser["Expression"] = [this](const SV& sv){return make_expr(sv);};
        parser["Assignment"] = [this](const SV& sv){return make_assign(sv);};
        parser["LogicOp"] = [](const SV& sv){return sv.token_to_string();};
        parser["Condition"] = [](const SV& sv){return sv.token_to_string();};
        parser["DeclareVar"] = [this](const SV& sv){return make_decl_var(sv);};
        parser["Statements"] = [](const SV& sv){return sv.token_to_string();};

        parser.enable_packrat_parsing();
        parser.parse(input);
        std::cout << input << std::endl;
    }
private:
    void make_decl_var(const SV& sv){
        for (int i = 0; i < sv.size(); ++i){
            std::string var = std::any_cast<std::string>(sv[i]);
            var_arrays[var] = 0;
        }
        // print all initialized variables
        for (const auto& [key, value] : var_arrays)std::cout << key << ": " << value << std::endl;
        return;
    }

    int make_expr(const SV& sv){
        int value = 0;
        std::string oper = "";
        for (int i = 0; i < sv.size(); ++i){
            std::string rhs = std::any_cast<std::string>(sv[i]);
            bool isDigits = std::all_of(rhs.begin(), rhs.end(), ::isdigit);
            if (isDigits && i == 0) value += std::stoi(rhs);
            else if (!isDigits && i == 0) value += var_arrays[rhs];
            else if (rhs == "+") oper = "+";
            else if (rhs == "-") oper = "-";
            else if (isDigits && oper == "+") value += std::stoi(rhs);
            else if (!isDigits && oper == "+") value += var_arrays[rhs];
            else if (isDigits && oper == "-") value -= std::stoi(rhs);
            else if (!isDigits && oper == "-") value -= var_arrays[rhs];
        }
        std::cout << "value = " << value << std::endl;
        return value;
    }

    int make_term(const SV& sv){
        int value = 0;
        std::string oper = "";
        for (int i = 0; i < sv.size(); ++i){
            std::string rhs = std::any_cast<std::string>(sv[i]);
            std::cout << rhs << std::endl;
        }
        std::cout << "value = " << value << std::endl;
        return value;
    }

    void make_assign(const SV& sv){
        std::string var = std::any_cast<std::string>(sv[0]);
        int value = std::any_cast<int>(sv[1]);
        var_arrays[var] = value;
        // print all variables
        for (const auto& [key, value]:var_arrays)std::cout << key << ": " << value << std::endl;
        return ;
    }
};

#endif