
#ifndef ABSTRACT_INTERPRETER_PARSER_HPP
#define ABSTRACT_INTERPRETER_PARSER_HPP


#include "peglib.h"
#include <assert.h>
#include <iostream>

// copyright 2024 Yi-Nung Tsao
class AbstractInterpreterParser{
    using SV = peg::SemanticValues;

    public:
        void parse(const std::string& input){
            // peg::parser parser(R"(
            //     Statements  <- (VariableDecl / Assignment /Comment)+

            //     Identifier  <- < [a-zA-Z_][a-zA-Z0-9_]* >
            //     Integer     <- < [+-]? [0-9]+ >
            //     VariableDecl <- 'int' (Identifier)+ ('=' Integer)? ';'

            //     Assignment <- Identifier '=' (Integer / Additive / Substractive / Multiplicative / Division ) ';'

            //     Additive <- Multiplicative '+' Additive / Multiplicative
            //     Substractive <- Multiplicative '-' Additive / Substractive / Multiplicative 
            //     Multiplicative <- Primary '*' Multiplicative / Primary
            //     Division <- Primary '/' Primary
            //     Primary <- '(' Additive ')' / Integer / Identifier

            //     ~Comment    <- '//' [^\n\r]* [ \n\r\t]*
            //     %whitespace <- [ \n\r\t]*
            // )");

            peg::parser parser(R"(
                Statements  <- (VariableDecl / Assignment / Additive / Multiplicative)+
                Identifier  <- < [a-zA-Z_][a-zA-Z0-9_]* >
                Integer     <- < [+-]? [0-9]+ >
                Arithemetics <- '+' / '-' / '*' / '/' / '%' 
                BinaryOperators <- '==' / '!=' / '>' / '>=' / '<' / '<='
                VariableDecl <- 'int' Identifier ('=' Integer)? ';'

                Assignment <- ( Identifier '=' Expression ) / ( Identifier'++' ) ';'
                Expression <- Additive / Substractive / Multiplicative / Division / Primary

                Additive <- Multiplicative '+' Additive / Multiplicative
                Substractive <- Multiplicative '-' Additive / Multiplicative
                Multiplicative <- Primary '*' Multiplicative / Primary
                Division <- Multiplicative '/' Identifier 
                Primary <- '(' Additive ')' / Integer / Identifier

                # If-Else, While-Loop
                If_Else <- ''
                While_Loop <- ''

                %whitespace <- [ \n\r\t]*
            )");

            assert(static_cast<bool>(parser) == true);

            // setup actions 
            parser["Integer"] = [](const SV& sv){
                return sv.token_to_number<int>();
            };
            parser["Additive"] = [](const SV &sv){
                switch (sv.choice())
                {
                case 0:
                    return std::any_cast<int>(sv[0]) + std::any_cast<int>(sv[1]);
                default:
                    return std::any_cast<int>(sv[0]);
                }
            };
            parser["Substractive"] = [](const SV &sv){
                switch (sv.choice())
                {
                case 0:
                    return std::any_cast<int>(sv[0]) - std::any_cast<int>(sv[1]);
                default:
                    return std::any_cast<int>(sv[0]);
                }
            };
            parser["Multiplicative"] = [](const SV &sv){
                switch (sv.choice())
                {
                case 0:
                    return std::any_cast<int>(sv[0]) * std::any_cast<int>(sv[1]);
                default:
                    return std::any_cast<int>(sv[0]);
                }
            };
            parser["Integer"] = [](const SV &sv){
                return sv.token_to_number<int>();
            };

            parser.enable_packrat_parsing();
            int val;
            parser.parse(input, val);

            std::cout << val << std::endl;
        }
};

#endif