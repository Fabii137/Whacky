#pragma once

#include <sstream>
#include <unordered_map>
#include <memory>
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

struct TypeInfo {
    VarType type;
    bool isValid = true;
    std::string errorMsg;
    
    static TypeInfo error(const std::string& msg) {
        return TypeInfo{VarType::Int, false, msg};
    }
    
    static TypeInfo valid(VarType t) {
        return TypeInfo{t, true, ""};
    }
};

class TypeChecker {
public:
    TypeChecker(const std::vector<Scope>& scopes);
    
    TypeInfo checkExpr(const NodeExpr* expr);
    TypeInfo checkTerm(const NodeTerm* term);
    TypeInfo checkBinExpr(const NodeBinExpr* binExpr);
    
private:
    const Var* lookupVar(const std::string& name);
    const std::vector<Scope>& m_Scopes;
};