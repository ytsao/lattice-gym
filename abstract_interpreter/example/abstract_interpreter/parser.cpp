#include <fstream>
#include <sstream>

#include "../../include/parser.hpp"
#include "../../include/ast.hpp"


int main(int argc, char* argv[]){
    int id = std::stoi(argv[1]);
    for (int i = id; i < id + 1; ++i){
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

        // std::cout << "Program content:\n" << input << std::endl;
        std::cout << "Program: " << filename << ".c" << std::endl;
        AbstractInterpreterParser AIParser;
        auto ast = AIParser.parse(input);
        printAST(ast);
    }

    return 0;
}