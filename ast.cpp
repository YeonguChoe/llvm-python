#include <iostream>
class ExpressionASTNode
{
public:
    virtual ~ExpressionASTNode()
    {
        std::cout << "AST has been deleted" << std::endl;
    };
};

class NumberExpressionASTNode : public ExpressionASTNode
{
    double value;

public:
    NumberExpressionASTNode(double value)
    {
        this->value = value;
    }
};

class VariableExpressionASTNode : public ExpressionASTNode
{
    std::string Name;

public:
    VariableExpressionASTNode(const std::string &Name)
    {
        this->Name = Name;
    }
};

class BinaryExpressionASTNode : public ExpressionASTNode
{
    char Operator;
    std::unique_ptr<ExpressionASTNode> LHS, RHS;

public:
    BinaryExpressionASTNode(char Operator, std::unique_ptr<ExpressionASTNode> LHS, std::unique_ptr<ExpressionASTNode> RHS)
    {
        this->Operator = Operator;
        this->LHS = std::move(LHS);
        this->RHS = std::move(RHS);
    }
};

class FunctionCallExpressionASTNode : public ExpressionASTNode
{
    std::string Callee;
    std::vector<std::unique_ptr<ExpressionASTNode>> Arguments;

public:
    FunctionCallExpressionASTNode(const std::string &Callee,
                                  std::vector<std::unique_ptr<ExpressionASTNode>> Arguments)
    {
        this->Callee = Callee;
        this->Arguments = std::move(Arguments);
    }
};

class SignatureASTNode
{
    std::string Name;
    std::vector<std::string> Arguments;

public:
    SignatureASTNode(const std::string &Name, std::vector<std::string> Arguments)
    {
        this->Name = Name;
        this->Arguments = std::move(Arguments);
    }
};

class FunctionASTNode
{
    std::unique_ptr<SignatureASTNode> Signature;
    std::unique_ptr<ExpressionASTNode> Body;

public:
    FunctionASTNode(std::unique_ptr<SignatureASTNode> Signature,
                    std::unique_ptr<ExpressionASTNode> Body)
    {
        this->Signature = std::move(Signature);
        this->Body = std::move(Body);
    }
};