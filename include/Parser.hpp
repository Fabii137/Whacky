#pragma once

#include <variant>
#include "Tokenizer.hpp"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

// struct NodeBinExprAdd {
//     NodeExpr left;
//     NodeExpr right;
// };

// struct NodeBinExprMul {
//     NodeExpr left;
//     NodeExpr right;
// };

// struct NodeBinExpr {
//     std::variant<NodeBinExprAdd, NodeBinExprMul> var;
// };

struct NodeExpr {
    std::variant<NodeExprIntLit, NodeExprIdent, /*NodeBinExpr*/> var;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr expr;
};

struct NodeStmtBye {
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtBye, NodeStmtLet> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts;
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::optional<NodeExpr> parseExpr();
    std::optional<NodeStmt> parseStmt();
    std::optional<NodeProg> parseProg();
private:
    std::optional<Token> peek(int offset = 0) const;
    Token consume();
private:
    size_t m_Index = 0;
    const std::vector<Token> m_Tokens;
};