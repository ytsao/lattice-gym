#include "ast.hpp"

class AbstractInterpreter {
public:
    void eval(const ASTNode& ast){
        // traverse the AST
        dfs(ast);
    }

    void dfs(const ASTNode& ast){
        if (ast.type == NodeType::DECLARATION){
            return;
        }
        
        if (ast.children.size() > 0){
            for (const auto& children: ast.children){
            
            }
        }
        else{

        }
        
    }
};