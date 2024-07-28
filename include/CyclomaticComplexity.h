#ifndef CYCLOMATIC_COMPLEXITY_H
#define CYCLOMATIC_COMPLEXITY_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <map>
#include <string>

class CyclomaticComplexityVisitor : public clang::RecursiveASTVisitor<CyclomaticComplexityVisitor> {
private:
    clang::ASTContext *Context;
    std::map<std::string, int> ComplexityMap;

public:
    explicit CyclomaticComplexityVisitor(clang::ASTContext *Context);
    bool VisitFunctionDecl(clang::FunctionDecl *FD);
    int countBranchingStatements(clang::Stmt *S);
    void printComplexity();
};

class CyclomaticComplexityASTConsumer : public clang::ASTConsumer {
private:
    CyclomaticComplexityVisitor Visitor;

public:
    explicit CyclomaticComplexityASTConsumer(clang::ASTContext *Context);
    void HandleTranslationUnit(clang::ASTContext &Context) override;
};

class CyclomaticComplexityFrontendAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef file) override;
};

#endif // CYCLOMATIC_COMPLEXITY_H