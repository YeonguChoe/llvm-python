#ifndef PARSER_H
#define PARSER_H

#include <memory>

namespace ASTNode {
    class ExpressionASTNode;
    class SignatureASTNode;
}

int getNextToken();

std::unique_ptr<ASTNode::ExpressionASTNode> LogError(const char *str);

std::unique_ptr<ASTNode::SignatureASTNode> LogErrorS(const char *str);


void MainLoop();

#endif
