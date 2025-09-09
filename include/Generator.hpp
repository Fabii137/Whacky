#pragma once

#include <sstream>
#include <unordered_map>
#include "Parser.hpp"

class Generator {
public:
    Generator(NodeProg prog);
    void generateExpr(const NodeExpr* expr);
    void generateStmt(const NodeStmt* stmt);
    std::string generateProg();
private:
    void push(const std::string& reg);
    void pop(const std::string& reg);
private:
    struct Var {
        size_t stackLoc;
    };

    const NodeProg m_Prog;
    std::stringstream m_Output;
    size_t m_StackSize = 0;
    std::unordered_map<std::string, Var> m_Vars {};
};