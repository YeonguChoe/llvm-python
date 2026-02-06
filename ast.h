#ifndef AST_H
#define AST_H

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <llvm/IR/Value.h>

namespace ASTNode {
    class ExpressionASTNode {
    public:
        virtual ~ExpressionASTNode() = default;

        // pure virtual
        // llvm::Value - constant, instruction, function, argument, global variable, block
        virtual llvm::Value *codegen() = 0;
    };

    class NumberExpressionASTNode : public ExpressionASTNode {
        double value;

    public:
        NumberExpressionASTNode(double value) : value(value) {
        }

        llvm::Value *codegen() override;
    };

    class VariableExpressionASTNode : public ExpressionASTNode {
        std::string Name;

    public:
        VariableExpressionASTNode(const std::string &Name) {
            this->Name = Name;
        }

        llvm::Value *codegen() override;
    };

    class BinaryExpressionASTNode : public ExpressionASTNode {
        char Operator;
        std::unique_ptr<ExpressionASTNode> LHS, RHS;

    public:
        BinaryExpressionASTNode(char Operator, std::unique_ptr<ExpressionASTNode> LHS,
                                std::unique_ptr<ExpressionASTNode> RHS) {
            this->Operator = Operator;
            this->LHS = std::move(LHS);
            this->RHS = std::move(RHS);
        }

        llvm::Value *codegen() override;
    };

    class FunctionCallExpressionASTNode : public ExpressionASTNode {
        std::string Callee;
        std::vector<std::unique_ptr<ExpressionASTNode> > Arguments;

    public:
        FunctionCallExpressionASTNode(const std::string &Callee,
                                      std::vector<std::unique_ptr<ExpressionASTNode> > Arguments) {
            this->Callee = Callee;
            this->Arguments = std::move(Arguments);
        }

        llvm::Value *codegen() override;
    };

    class IfExpressionASTNode : public ExpressionASTNode {
        // Each of them are AST nodes
        std::unique_ptr<ExpressionASTNode> Condition, Then, Else;

    public:
        IfExpressionASTNode(std::unique_ptr<ExpressionASTNode> Condition, std::unique_ptr<ExpressionASTNode> Then,
                            std::unique_ptr<ExpressionASTNode> Else) : Condition(std::move(Condition)),
                                                                       Then(std::move(Then)), Else(std::move(Else)) {
        }

        llvm::Value *codegen() override;
    };

    class ForExpressionASTNode : public ExpressionASTNode {
        std::string VariableName;
        std::unique_ptr<ExpressionASTNode> Start, End, Step, Body;

    public:
        ForExpressionASTNode(const std::string &VariableName,
                             std::unique_ptr<ExpressionASTNode> Start,
                             std::unique_ptr<ExpressionASTNode> End,
                             std::unique_ptr<ExpressionASTNode> Step,
                             std::unique_ptr<ExpressionASTNode> Body
        ) : VariableName(VariableName),
            Start(std::move(Start)),
            End(std::move(End)),
            Step(std::move(Step)),
            Body(std::move(Body)) {
        }

        llvm::Value *codegen() override;
    };

    class SignatureASTNode {
        std::string Name;
        std::vector<std::string> Arguments;

    public:
        SignatureASTNode(const std::string &Name, std::vector<std::string> Arguments) {
            this->Name = Name;
            this->Arguments = std::move(Arguments);
        }

        llvm::Function *codegen();

        const std::string &getName() const { return Name; }
    };

    class FunctionASTNode {
        std::unique_ptr<SignatureASTNode> Signature;
        std::unique_ptr<ExpressionASTNode> Body;

    public:
        FunctionASTNode(std::unique_ptr<SignatureASTNode> Signature,
                        std::unique_ptr<ExpressionASTNode> Body) {
            this->Signature = std::move(Signature);
            this->Body = std::move(Body);
        }

        llvm::Function *codegen();
    };
}

#endif
