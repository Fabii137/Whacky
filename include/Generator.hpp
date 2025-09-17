#pragma once

#include <sstream>
#include <unordered_map>
#include "Parser.hpp"

enum class VarType {
    Int,
    Bool,
    String,
};

struct Var {
    size_t size = 8;
    VarType type;
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
    VarType findType(const NodeExpr* expr);
    void generateBinExpr(const NodeBinExpr* binExpr);
    void generateExpr(const NodeExpr* expr);
    void generateScope(const NodeScope* scope);
    void generateMaybePred(const NodeMaybePred* pred, const std::string& endLabel);
    void generateStmt(const NodeStmt* stmt);
    std::string generateProg(); 
private:
    void push(const std::string& reg, size_t size = 8);
    void pop(const std::string& reg, size_t size = 8);

    void enterScope();
    void leaveScope();
    std::string createLabel();
    std::string findStringLiteral(const std::string& value);
    Var* lookupVar(const std::string& name);
    void declareVar(const std::string& name, VarType type);
private:
    const NodeProg m_Prog;
    std::stringstream m_Output;
    std::stringstream m_Data;
    std::unordered_map<std::string, std::string> m_StringLiterals;
    size_t m_StackSize = 0;
    std::vector<Scope> m_Scopes {};
    size_t m_LabelCount = 0;
};