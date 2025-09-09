#include "Parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_Tokens(std::move(tokens)), m_Allocator(1024 * 1024 * 4) /* 4 mb */ {

}

std::optional<NodeTerm*> Parser::parseTerm() {
    if(peek().has_value() && peek().value().type == TokenType::int_lit) {
        NodeTermIntLit* termIntLit = m_Allocator.alloc<NodeTermIntLit>();
        termIntLit->int_lit = consume();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termIntLit;
        return term;
    } else if (peek().has_value() && peek().value().type == TokenType::ident) {
        NodeTermIdent* termIdent = m_Allocator.alloc<NodeTermIdent>();
        termIdent->ident = consume();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termIdent;
        return term;
    } else {
        return {};
    }
}

std::optional<NodeExpr*> Parser::parseExpr() {
    if(auto term = parseTerm()) {
        if(peek().has_value() && peek().value().type == TokenType::plus) {
            NodeBinExpr* binExpr = m_Allocator.alloc<NodeBinExpr>();
            if(peek().has_value() && peek().value().type == TokenType::plus) {
                NodeBinExprAdd* binExprAdd = m_Allocator.alloc<NodeBinExprAdd>();
                NodeExpr* leftExpr = m_Allocator.alloc<NodeExpr>();
                leftExpr->var = term.value();
                binExprAdd->left = leftExpr;
                consume(); // +
                if(auto right = parseExpr()) {
                    binExprAdd->right = right.value();
                    binExpr->var = binExprAdd;

                    NodeExpr* expr = m_Allocator.alloc<NodeExpr>();
                    expr->var = binExpr;
                    return expr;
                } else {
                    std::cerr << "Expected expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                NodeExpr* expr = m_Allocator.alloc<NodeExpr>();
            }
        }
    } 
}

std::optional<NodeStmt*> Parser::parseStmt() {
    if(peek().has_value() && peek().value().type == TokenType::bye && peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
        consume(); // exit
        consume(); // open paren
        NodeStmtBye* stmtBye = m_Allocator.alloc<NodeStmtBye>();
        if(auto expr = parseExpr()) {
            stmtBye->expr = expr.value();
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

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = stmtBye;
        return stmt;
    } else if (
        peek().has_value() && peek().value().type == TokenType::let 
        && peek(1).has_value() && peek(1).value().type == TokenType::ident 
        && peek(2).has_value() && peek(2).value().type == TokenType::eq
    ) {
        consume(); // let
        NodeStmtLet* stmtLet = m_Allocator.alloc<NodeStmtLet>();
        stmtLet->ident = consume();
        consume(); // eq

        if(auto expr = parseExpr()) {
            stmtLet->expr = expr.value();
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

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = stmtLet;
        return stmt;
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