#pragma once

#include <variant>
#include "Tokenizer.hpp"
#include "ArenaAllocator.hpp"

struct NodeExpr;
struct NodeStmt;
struct NodeMaybePred;

struct NodeBinExprAdd {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprSub {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprMul {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprDiv {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMul*, NodeBinExprDiv*> var;
};

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeMaybePredBut {
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeMaybePred*> pred;
};

struct NodeMaybePredNah {
    NodeScope* scope;
};

struct NodeMaybePred {
    std::variant<NodeMaybePredBut*, NodeMaybePredNah*> var;
};

struct NodeStmtMaybe {
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeMaybePred*> pred;
};

struct NodeStmtBye {
    NodeExpr* expr;
};

struct NodeStmtGimme {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmtAssignment {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtBye*, NodeStmtGimme*, NodeScope*, NodeStmtMaybe*, NodeStmtAssignment*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::optional<NodeTerm*> parseTerm();
    std::optional<NodeExpr*> parseExpr(const int minPrec = 0);
    std::optional<NodeScope*> parseScope();
    std::optional<NodeMaybePred*> parseMaybePred();
    std::optional<NodeStmt*> parseStmt();
    std::optional<NodeProg> parseProg();
private:
    std::optional<Token> peek(const size_t offset = 0) const;
    Token consume();
    Token tryConsume(const TokenType type, const std::string& errMessage);
    std::optional<Token> tryConsume(const TokenType type);
private:
    size_t m_Index = 0;
    const std::vector<Token> m_Tokens;
    ArenaAllocator m_Allocator;
};