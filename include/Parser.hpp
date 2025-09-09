#pragma once

#include <variant>
#include "Tokenizer.hpp"
#include "ArenaAllocator.hpp"

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr* left;
    NodeExpr* right;
};

// struct NodeBinExprMul {
//     NodeExpr* left;
//     NodeExpr* right;
// };

struct NodeBinExpr {
    NodeBinExprAdd* add;
};

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtBye {
    NodeExpr* expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtBye*, NodeStmtLet*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::optional<NodeBinExpr*> parseBinExpr();
    std::optional<NodeTerm*> parseTerm();
    std::optional<NodeExpr*> parseExpr();
    std::optional<NodeStmt*> parseStmt();
    std::optional<NodeProg> parseProg();
private:
    std::optional<Token> peek(int offset = 0) const;
    Token consume();
    Token tryConsume(TokenType type, const std::string& errMessage);
    std::optional<Token> tryConsume(TokenType type);
private:
    size_t m_Index = 0;
    const std::vector<Token> m_Tokens;
    ArenaAllocator m_Allocator;
};