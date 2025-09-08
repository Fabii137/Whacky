#include "Parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_Tokens(std::move(tokens)) {

}

std::optional<NodeExpr> Parser::parseExpr() {
    if(peek().has_value() && peek().value().type == TokenType::int_lit) {
        return NodeExpr{ .var = NodeExprIntLit{ .int_lit = consume() } };
    } else if (peek().has_value() && peek().value().type == TokenType::ident) {
        return NodeExpr{ .var= NodeExprIdent{ .ident = consume() } };
    } else {
        return {};
    }
}

std::optional<NodeStmt> Parser::parseStmt() {
    if(peek().has_value() && peek().value().type == TokenType::bye && peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
        consume(); // exit
        consume(); // open paren
        NodeStmtBye stmtBye;
        if(auto nodeExpr = parseExpr()) {
            stmtBye =  { .expr = nodeExpr.value() };
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        if(peek().has_value() && peek().value().type == TokenType::close_paren) {
            consume();
        } else {
            std::cerr << "Expected ')'" << std::endl;
            exit(EXIT_FAILURE);
        }

        if(peek().has_value() && peek().value().type == TokenType::semi) {
            consume();
        } else {
            std::cerr << "Expected ';'" << std::endl;
            exit(EXIT_FAILURE);
        }

        return NodeStmt{ .var = stmtBye };
    } else if (
        peek().has_value() && peek().value().type == TokenType::let 
        && peek(1).has_value() && peek(1).value().type == TokenType::ident 
        && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
        consume(); // let
        NodeStmtLet stmtLet = { .ident = consume() };
        consume(); // eq

        if(auto expr = parseExpr()) {
            stmtLet.expr = expr.value();
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        if(peek().has_value() && peek().value().type == TokenType::semi) {
            consume(); // ;
        } else {
            std::cerr << "Expected ';'" << std::endl;
            exit(EXIT_FAILURE);
        }

        return NodeStmt{ .var = stmtLet };
    } else {
        return {};
    }
}

std::optional<NodeProg> Parser::parseProg() {
    NodeProg prog;
    while(peek().has_value()) {
        if(auto stmt = parseStmt()) {
            prog.stmts.push_back(stmt.value());
        } else {
            std::cerr << "Invalid statement" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return prog;
}


std::optional<Token> Parser::peek(int offset /*=0*/) const {
    if(m_Index + offset >= m_Tokens.size()) {
        return {};
    }

    return m_Tokens.at(m_Index + offset);
}

Token Parser::consume() {
    return m_Tokens.at(m_Index++);
}