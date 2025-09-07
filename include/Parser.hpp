#pragma once

#include "Tokenizer.hpp"

struct NodeExpr {
    Token int_lit;
};

struct NodeBye {
    NodeExpr expr;
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::optional<NodeExpr> parseExpr();
    std::optional<NodeBye> parse();
private:
    std::optional<Token> peak(int steps = 1) const;
    Token consume();
private:
    size_t m_Index = 0;
    const std::vector<Token> m_Tokens;
};