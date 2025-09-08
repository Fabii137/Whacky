#include "Parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_Tokens(std::move(tokens)) {

}

std::optional<NodeExpr> Parser::parseExpr() {
    if(peek().has_value() && peek().value().type == TokenType::int_lit) {
        return NodeExpr{ .int_lit = consume() };
    } else {
        return {};
    }
}

std::optional<NodeBye> Parser::parse() {
    std::optional<NodeBye> byeNode;
    while(peek().has_value()) {
        if(peek().value().type == TokenType::bye) {
            consume();
            if(auto nodeExpr = parseExpr()) {
                byeNode = NodeBye { .expr = nodeExpr.value() };
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            if(peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
    m_Index = 0;
    return byeNode;
}


std::optional<Token> Parser::peek(int steps) const {
    if(m_Index + steps > m_Tokens.size()) {
        return {};
    }

    return m_Tokens.at(m_Index);
}

Token Parser::consume() {
    return m_Tokens.at(m_Index++);
}