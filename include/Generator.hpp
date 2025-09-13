#pragma once

#include <sstream>
#include <unordered_map>
#include "Parser.hpp"

struct Var {
    size_t stackLoc;
};

struct Scope {
    std::unordered_map<std::string, Var> vars;
    size_t stackStart;
};

class Generator {
public:
    Generator(NodeProg prog);
    void generateTerm(const NodeTerm* term);
    void generateBinExpr(const NodeBinExpr* binExpr);
    void generateExpr(const NodeExpr* expr);
    void generateScope(const NodeScope* scope);
    void generateMaybePred(const NodeMaybePred* pred, const std::string& endLabel);
    void generateStmt(const NodeStmt* stmt);
    std::string generateProg(); 
private:
    void push(const std::string& reg);
    void pop(const std::string& reg);

    void enterScope();
    void leaveScope();
    std::string createLabel();
    Var* lookupVar(const std::string& name);
    void declareVar(const std::string& name, Var var);
private:
    const NodeProg m_Prog;
    std::stringstream m_Output;
    size_t m_StackSize = 0;
    std::vector<Scope> m_Scopes {};
    int m_LabelCount = 0;
};