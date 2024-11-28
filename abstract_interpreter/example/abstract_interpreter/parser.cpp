
#include "../../include/parser.hpp"
#include "../../include/ast.hpp"

int main(){
    std::shared_ptr<ExprNode> root;
    AbstractInterpreterParser AIParser;
    // AIParser.parse(" (-1+2)*3", root);
    AIParser.parse("int a1, a2 ; a1=4+3;a2=2*(a1-10)+8;");
    // AIParser.parse("-1+(1+2)*3- -1");
    return 0;
}