#pragma once

#include <sstream>
#include <unordered_map>
#include <memory>
#include "Parser.hpp"


enum class VarType {
    Number,
    Bool,
    String,
};

inline std::string getTypeName(VarType type) {
    switch (type) {
        case VarType::Number: return "number";
        case VarType::String: return "str";
        case VarType::Bool: return "bool";
        default: return "unknown";
    }
}
inline VarType tokenTypeToVarType(TokenType type) {
    switch (type) {
        case TokenType::type_number: return VarType::Number;
        case TokenType::type_string: return VarType::String;
        case TokenType::type_bool: return VarType::Bool;
        default:
            throw std::runtime_error("Invalid type token");
    }
}

struct Var {
    size_t size = 8;
    VarType type;
    size_t stackLoc;
    bool isParam;
};

struct Thingy {
    std::vector<VarType> paramTypes;
    VarType returnType;
    std::string label;
};

struct Scope {
    std::unordered_map<std::string, Var> vars;
    std::unordered_map<std::string, Thingy> functions;
    size_t stackStart;
};

struct TypeInfo {
    VarType type;
    bool isValid = true;
    std::string errorMsg;
    
    static TypeInfo error(const std::string& msg) {
        return TypeInfo{VarType::Number, false, msg};
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
    const Thingy* lookupThingy(const std::string& name);
    const std::vector<Scope>& m_Scopes;
};