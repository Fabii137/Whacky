#pragma once

#include "Parser.hpp"
#include "TypeChecker.hpp"
#include "OperationGenerator.hpp"

class Generator {
public:
    Generator(NodeProg prog);
    
    void generateTerm(const NodeTerm* term);
    void generateExpr(const NodeExpr* expr);
    void generateBinExpr(const NodeBinExpr* binExpr);
    void generateScope(const NodeScope* scope);
    void generateMaybePred(const NodeMaybePred* pred, const std::string& endLabel);
    void generateStmt(const NodeStmt* stmt);
    std::string generateProg();
    
private:
    void push(const std::string& reg, size_t size = 8);
    void pop(const std::string& reg, size_t size = 8);
    
    void enterScope();
    void leaveScope();
    
    void declareVar(const std::string& name, VarType type);
    Var* lookupVar(const std::string& name);
    
    std::string createLabel(const std::string& name = "label");
    std::optional<std::string> findStringLiteral(const std::string& value, const bool& create = true);
    static const std::string escapeString(const std::string& input);
    void generateVariableLoad(const Var* var);
    void generateVariableStore(const Var* var);
    
    void error(const std::string& msg);
    
private:
    const NodeProg m_Prog;
    std::stringstream m_Output;
    std::stringstream m_Data;
    std::unordered_map<std::string, std::string> m_StringLiterals;
    size_t m_StackSize = 0;
    std::vector<Scope> m_Scopes;
    size_t m_LabelCount = 0;
    
    std::unique_ptr<TypeChecker> m_TypeChecker;
    std::unique_ptr<OperationGenerator> m_OpGenerator;
};