#pragma once

#include <variant>
#include "Tokenizer.hpp"
#include "ArenaAllocator.hpp"

struct NodeExpr;
struct NodeStmt;
struct NodeMaybePred;

struct NodeBinExprOr {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprAnd {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprNeq {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprEq {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprGe {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprGt {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprLe {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeBinExprLt {
    NodeExpr* left;
    NodeExpr* right;
};

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
    std::variant<NodeBinExprOr*, NodeBinExprAnd*, NodeBinExprNeq*, NodeBinExprEq*, NodeBinExprGe*, NodeBinExprGt*, NodeBinExprLe*, NodeBinExprLt*, NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMul*, NodeBinExprDiv*> var;
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
    std::optional<Token> peek(const int offset = 0) const;
    Token consume();
    Token tryConsumeErr(const TokenType& type);
    std::optional<Token> tryConsume(const TokenType& type);
    void errorExpected(const std::string& msg) const;
private:
    size_t m_Index = 0;
    const std::vector<Token> m_Tokens;
    ArenaAllocator m_Allocator;
};