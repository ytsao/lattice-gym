#include <fstream>
#include <sstream>

#include "../../include/parser.hpp"
#include "../../include/ast.hpp"


int main(){
    // std::string input = R"(
    //     int x, y ;
    //     int x = 4;
    //     int y = 3;
    //     x = 2 * (y - 10);
    //     if (x == 10) {
    //         if (y == 10){
    //             y = 3;
    //             x = 5;
    //         }
    //     }
    //     while(x != 0){
    //         int z = 10;
    //     }
    // )";
    for (int i = 0; i < 1; ++i){
        std::string filename = (i < 10 ? "0" : "") + std::to_string(i); // Add "0" prefix if i < 10
        std::ifstream f("./tests/" + filename + ".c");
        if (!f.is_open()){
            std::cerr << "[ERROR] file cannot open!" << std::endl;
            return -1;
        }

        std::ostringstream buffer;
        buffer << f.rdbuf();
        std::string input = buffer.str();
        f.close();

        std::cout << "Program content:\n" << input << std::endl;

        AbstractInterpreterParser AIParser;
        auto ast = AIParser.parse(input);
        printAST(ast);
    }

    return 0;
}