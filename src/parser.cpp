#include "Parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_Tokens(std::move(tokens)), m_Allocator(1024 * 1024 * 4) /* 4 mb */ {

}

std::optional<NodeTerm*> Parser::parseTerm() {
    if(auto intLit = tryConsume(TokenType::int_lit)) {
        NodeTermIntLit* termIntLit = m_Allocator.alloc<NodeTermIntLit>();
        termIntLit->int_lit = intLit.value();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termIntLit;
        return term;
    } 

    if (auto ident = tryConsume(TokenType::ident)) {
        NodeTermIdent* termIdent = m_Allocator.alloc<NodeTermIdent>();
        termIdent->ident = ident.value();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termIdent;
        return term;
    }

    if (auto openParen = tryConsume(TokenType::open_paren)) {
        auto expr = parseExpr();
        if(!expr.has_value()) {
            std::cerr << "Expected expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        tryConsume(TokenType::close_paren, "Expected ')'");
        NodeTermParen* termParen = m_Allocator.alloc<NodeTermParen>();
        termParen->expr = expr.value();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termParen;
        return term;
    }

    return {};
}

std::optional<NodeExpr*> Parser::parseExpr(const int minPrec /*=0*/) {
    auto termLeft = parseTerm();
    if(!termLeft.has_value()) {
        return {};
    }

    auto exprLeft = m_Allocator.alloc<NodeExpr>();
    exprLeft->var = termLeft.value();

    while(true) {
        std::optional<Token> currToken = peek();
        std::optional<int> prec;
        if(!currToken.has_value()) {
            break;
        }

        prec = binPrec(currToken->type);
        if(!prec.has_value() || prec.value() < minPrec) {
            break;
        }

        const auto [type, value] = consume(); // operator
        const int nextMinPrec = prec.value() + 1;
        auto exprRight = parseExpr(nextMinPrec);
        if(!exprRight.has_value()) {
            std::cerr << "Unable to parse expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        auto binExpr = m_Allocator.alloc<NodeBinExpr>();
        auto exprLeft2 = m_Allocator.alloc<NodeExpr>();

        switch(type) {
            case TokenType::plus: {
                auto binExprAdd = m_Allocator.alloc<NodeBinExprAdd>();
                exprLeft2->var = exprLeft->var;
                binExprAdd->left = exprLeft2;
                binExprAdd->right = exprRight.value();
                binExpr->var = binExprAdd;
                break;
            }
            case TokenType::minus: {
                auto binExprSub = m_Allocator.alloc<NodeBinExprSub>();
                exprLeft2->var = exprLeft->var;
                binExprSub->left = exprLeft2;
                binExprSub->right = exprRight.value();
                binExpr->var = binExprSub;
                break;
            }
            case TokenType::star: {
                auto binExprMul = m_Allocator.alloc<NodeBinExprMul>();
                exprLeft2->var = exprLeft->var;
                binExprMul->left = exprLeft2;
                binExprMul->right = exprRight.value();
                binExpr->var = binExprMul;
                break;
            }
            case TokenType::fslash: {
                auto binExprDiv = m_Allocator.alloc<NodeBinExprDiv>();
                exprLeft2->var = exprLeft->var;
                binExprDiv->left = exprLeft2;
                binExprDiv->right = exprRight.value();
                binExpr->var = binExprDiv;
                break;
            }
            default:
                break; 
        }
       
        exprLeft->var = binExpr;
    }

    return exprLeft;
}

std::optional<NodeScope*> Parser::parseScope() {
    if(!tryConsume(TokenType::open_curly).has_value()) {
        return {};
    }

    NodeScope* scope = m_Allocator.alloc<NodeScope>();
    while(auto stmt = parseStmt()) {
        scope->stmts.push_back(stmt.value());
    }

    tryConsume(TokenType::close_curly, "Expected '}'");

    return scope;
}

std::optional<NodeStmt*> Parser::parseStmt() {
    if(peek().has_value() && peek().value().type == TokenType::bye && peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
        consume(); // bye
        consume(); // open paren
        NodeStmtBye* stmtBye = m_Allocator.alloc<NodeStmtBye>();
        if(const auto expr = parseExpr()) {
            stmtBye->expr = expr.value();
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        tryConsume(TokenType::close_paren, "Expected ')'");
        tryConsume(TokenType::semi, "Expected ';'");

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = stmtBye;
        return stmt;
    }
    
    if (
        peek().has_value() && peek().value().type == TokenType::let 
        && peek(1).has_value() && peek(1).value().type == TokenType::ident 
        && peek(2).has_value() && peek(2).value().type == TokenType::eq
    ) {
        consume(); // let
        NodeStmtLet* stmtLet = m_Allocator.alloc<NodeStmtLet>();
        stmtLet->ident = consume();
        consume(); // eq

        if(const auto expr = parseExpr()) {
            stmtLet->expr = expr.value();
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        tryConsume(TokenType::semi, "Expected ';'");

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = stmtLet;
        return stmt;
    }
    
    if (peek().has_value() && peek().value().type == TokenType::open_curly) {
        if(const auto scope = parseScope()) {
            NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
            stmt->var = scope.value();
            return stmt;
        }

        std::cerr << "Invalid scope" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    if (auto maybe = tryConsume(TokenType::maybe)) {
        tryConsume(TokenType::open_paren, "Expected '('");
        NodeStmtMaybe* stmtMaybe = m_Allocator.alloc<NodeStmtMaybe>();
        if(const auto expr = parseExpr()) {
            stmtMaybe->expr = expr.value();
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        tryConsume(TokenType::close_paren, "Expected ')'");

        if(const auto scope = parseScope()) {
            stmtMaybe->scope = scope.value();
        } else {
            std::cerr << "Invalid scope" << std::endl;
            exit(EXIT_FAILURE);
        }

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = stmtMaybe;
        return stmt;
    }

    return {};
}

std::optional<NodeProg> Parser::parseProg() {
    NodeProg prog;
    while(peek().has_value()) {
        if(const auto stmt = parseStmt()) {
            prog.stmts.push_back(stmt.value());
        } else {
            std::cerr << "Invalid statement" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return prog;
}


std::optional<Token> Parser::peek(const size_t offset /*=0*/) const {
    if(m_Index + offset >= m_Tokens.size()) {
        return {};
    }

    return m_Tokens.at(m_Index + offset);
}

Token Parser::consume() {
    return m_Tokens.at(m_Index++);
}

Token Parser::tryConsume(const TokenType type, const std::string& errMessage) {
    if(peek().has_value() && peek().value().type == type) {
        return consume();
    }

    std::cerr << errMessage << std::endl;
    exit(EXIT_FAILURE);
}

std::optional<Token> Parser::tryConsume(const TokenType type) {
    if(peek().has_value() && peek().value().type == type) {
        return consume();
    }

    return {};
}