#pragma once

#include <variant>
#include <unordered_map>
#include "Tokenizer.hpp"
#include "ArenaAllocator.hpp"

enum class BinOp {
    Or, And, Band, Bor, Xor, Neq, Eq, Ge, Gt, Le, Lt, Add, Sub, Mul, Div 
};

inline static const std::unordered_map<TokenType, BinOp> tokenTypeToBinOp = {
    { TokenType::_or, BinOp::Or },
    { TokenType::_and, BinOp::And },
    { TokenType::band, BinOp::Band },
    { TokenType::bor, BinOp::Bor },
    { TokenType::_xor, BinOp::Xor },
    { TokenType::neq, BinOp::Neq },
    { TokenType::eqeq, BinOp::Eq },
    { TokenType::ge, BinOp::Ge },
    { TokenType::gt, BinOp::Gt },
    { TokenType::le, BinOp::Le },
    { TokenType::lt, BinOp::Lt },
    { TokenType::plus, BinOp::Add },
    { TokenType::minus, BinOp::Sub },
    { TokenType::star, BinOp::Mul },
    { TokenType::fslash, BinOp::Div },
};

struct NodeExpr;
struct NodeStmt;
struct NodeMaybePred;
struct NodeType;

struct NodeBinExpr {
   BinOp op;
   NodeExpr* left;
   NodeExpr* right;
};

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermBool {
    Token _bool;
};

struct NodeTermString {
    Token string;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeTermCall {
    Token ident;
    std::vector<NodeExpr*> args;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermBool*, NodeTermString*, NodeTermIdent*, NodeTermParen*, NodeTermCall*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeMaybePredBut {
    NodeExpr* expr{};
    NodeScope* scope{};
    std::optional<NodeMaybePred*> pred;
};

struct NodeMaybePredNah {
    NodeScope* scope;
};

struct NodeMaybePred {
    std::variant<NodeMaybePredBut*, NodeMaybePredNah*> var;
};

struct NodeStmtMaybe {
    NodeExpr* expr{};
    NodeScope* scope{};
    std::optional<NodeMaybePred*> pred;
};

struct NodeStmtYell {
    NodeExpr* expr;
};

struct NodeParam {
    Token name;
    NodeType* type;
};

struct NodeStmtThingy {
    Token name;
    std::vector<NodeParam*> params;
    NodeType* returnType;
    NodeScope* scope;
};

struct NodeStmtGimmeback {
    NodeExpr* expr;
};

struct NodeStmtFour {
    Token ident;
    NodeExpr* start{};
    NodeExpr* end{};
    NodeScope* scope{};
};

struct NodeStmtWhy {
    NodeExpr* expr;
    NodeScope* scope;
};

struct NodeStmtBye {
    NodeExpr* expr;
};

struct NodeType {
    TokenType type;  // type_number, type_string, type_bool
};

struct NodeStmtGimme {
    Token ident;
    NodeType* type;
    NodeExpr* expr{};
};

struct NodeStmtAssignment {
    Token ident;
    NodeExpr* expr{};
};

struct NodeStmt {
    std::variant<NodeStmtBye*, NodeStmtGimme*, NodeScope*, NodeStmtMaybe*, NodeStmtYell*, NodeStmtThingy*, NodeStmtGimmeback*, NodeStmtFour*, NodeStmtWhy*, NodeStmtAssignment*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::optional<NodeTerm*> parseTerm();
    std::optional<NodeExpr*> parseExpr(int minPrec = 0);
    std::optional<NodeScope*> parseScope();
    std::optional<NodeMaybePred*> parseMaybePred();
    std::optional<NodeStmt*> parseStmt();
    NodeProg parseProg();
private:
    std::optional<Token> peek(int offset = 0) const;
    Token consume();
    Token tryConsumeErr(const TokenType& type);
    std::optional<Token> tryConsume(const TokenType& type);
    void errorExpected(const std::string& msg) const;
private:
    size_t m_Index = 0;
    const std::vector<Token> m_Tokens;
    ArenaAllocator m_Allocator;
    int m_FunctionDepth = 0;
};