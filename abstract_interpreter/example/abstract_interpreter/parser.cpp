
// (1) Include the header file
#include "../../include/parser.hpp"

int main(){
    AbstractInterpreterParser AIParser;
    AIParser.parse(" (-1+2)*3");
    // AIParser.parse("-1+(1+2)*3- -1");
    return 0;
}