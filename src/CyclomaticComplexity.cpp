#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream> // Include fstream for file operations

#include "clang/AST/Expr.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <string>

using namespace clang;

class CyclomaticComplexityVisitor : public RecursiveASTVisitor<CyclomaticComplexityVisitor> {
private:
    ASTContext *context;
    CompilerInstance& instance;
    DiagnosticsEngine& d;
    unsigned int remarkID;
    std::map<std::string, int> ComplexityMap;

    bool isInHeader(Decl *decl) {
        auto loc = decl->getLocation();
        auto floc = context->getFullLoc(loc);
        if (floc.isInSystemHeader()) return true;
        auto entry = floc.getFileEntry()->getName();
        if (entry.ends_with(".h") || entry.ends_with(".hpp")) {
            return true;
        }
        return false;
    }

    void reportCyclomaticComplexity(FunctionDecl *func, int complexity) {
        auto loc = context->getFullLoc(func->getLocation());
        d.Report(loc, remarkID) << complexity;
    }

    int calculateCyclomaticComplexity(const Stmt *stmt) {
        if (!stmt) {
            std::cout << "Null statement encountered!\n";
            return 0;
        }
        int complexity = 1; // Start with 1 for the function itself
        complexity += countBranchingStatements(stmt);
        return complexity;
    }

    int countBranchingStatements(const Stmt *stmt) {
        int count = 0;
        if (isa<IfStmt>(stmt) || isa<SwitchStmt>(stmt) || isa<ForStmt>(stmt) || 
            isa<WhileStmt>(stmt) || isa<DoStmt>(stmt) || isa<ConditionalOperator>(stmt)) {
            count++;
        }
        for (auto child : stmt->children()) {
            if (child) {
                count += countBranchingStatements(child);
            }
        }
        return count;
    }

public:
    explicit CyclomaticComplexityVisitor(ASTContext *context, CompilerInstance& instance)
        : context(context), instance(instance), d(instance.getDiagnostics()) {
        remarkID = d.getCustomDiagID(DiagnosticsEngine::Remark, "Cyclomatic Complexity: %0");
    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        if (isInHeader(func))
            return true;

        if (func->hasBody()) {
            int complexity = calculateCyclomaticComplexity(func->getBody());
            ComplexityMap[func->getNameAsString()] = complexity;
            reportCyclomaticComplexity(func, complexity);
        }
        return true;
    }

    void writeComplexityToFile(const std::string &filename) {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        for (const auto &entry : ComplexityMap) {
            outFile << "Function: " << entry.first << ", Cyclomatic Complexity: " << entry.second << std::endl;
        }
        outFile.close();
    }
};

class CyclomaticComplexityConsumer : public ASTConsumer {
    CompilerInstance& instance;
    CyclomaticComplexityVisitor visitor;

public:
    CyclomaticComplexityConsumer(CompilerInstance& instance)
        : instance(instance), visitor(&instance.getASTContext(), instance) {}

    virtual void HandleTranslationUnit(ASTContext &context) override {
        visitor.TraverseDecl(context.getTranslationUnitDecl());
        visitor.writeComplexityToFile("results.cy");
    }
};

class CyclomaticComplexityAction : public PluginASTAction {
protected:
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& instance, llvm::StringRef) override {
        return std::make_unique<CyclomaticComplexityConsumer>(instance);
    }

    virtual bool ParseArgs(const CompilerInstance&, const std::vector<std::string>&) override {
        return true;
    }

    virtual PluginASTAction::ActionType getActionType() override {
        return PluginASTAction::AddAfterMainAction;
    }
};

static FrontendPluginRegistry::Add<CyclomaticComplexityAction> X("cyclomatic-complexity", "Calculate cyclomatic complexity of functions");
