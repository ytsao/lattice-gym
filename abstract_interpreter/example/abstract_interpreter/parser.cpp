
#include "../../include/parser.hpp"
#include "../../include/ast.hpp"

int main(){
    std::string input = R"(
        int x, y ;
        int x = 4;
        int y = 3;
        x = 2 * (y - 10);
        if (x == 10) {
            if (y == 10){
                y = 3;
                x = 5;
            }
        }
        while(x != 0){
            int z = 10;
        }
    )";
    
    AbstractInterpreterParser AIParser;
    auto ast = AIParser.parse(input);
    printAST(ast);
    // AIParser.parse("-1+(1+2)*3- -1");
    

    // std::cout << "Result: " << root->evaluate() << std::endl;
    return 0;
}