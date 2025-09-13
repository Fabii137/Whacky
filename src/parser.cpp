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
                auto add = m_Allocator.alloc<NodeBinExprAdd>();
                exprLeft2->var = exprLeft->var;
                add->left = exprLeft2;
                add->right = exprRight.value();
                binExpr->var = add;
                break;
            }
            case TokenType::minus: {
                auto sub = m_Allocator.alloc<NodeBinExprSub>();
                exprLeft2->var = exprLeft->var;
                sub->left = exprLeft2;
                sub->right = exprRight.value();
                binExpr->var = sub;
                break;
            }
            case TokenType::star: {
                auto mul = m_Allocator.alloc<NodeBinExprMul>();
                exprLeft2->var = exprLeft->var;
                mul->left = exprLeft2;
                mul->right = exprRight.value();
                binExpr->var = mul;
                break;
            }
            case TokenType::fslash: {
                auto div = m_Allocator.alloc<NodeBinExprDiv>();
                exprLeft2->var = exprLeft->var;
                div->left = exprLeft2;
                div->right = exprRight.value();
                binExpr->var = div;
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

std::optional<NodeMaybePred*> Parser::parseMaybePred() {
    if(tryConsume(TokenType::but)) {
        tryConsume(TokenType::open_paren, "Expected '('");
        const auto but = m_Allocator.alloc<NodeMaybePredBut>();
        if(const auto expr = parseExpr()) {
            but->expr = expr.value();
        } else {
            std::cerr << "Expected expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        tryConsume(TokenType::close_paren, "Expected ')'");

        if(const auto scope = parseScope()) {
            but->scope = scope.value();
        
        } else {
            std::cerr << "Expected scope" << std::endl;
            exit(EXIT_FAILURE);
        }

        but->pred = parseMaybePred();
        const auto pred = m_Allocator.alloc<NodeMaybePred>();
        pred->var = but;
        return pred;
    }

    if(tryConsume(TokenType::nah)) {
        const auto nah = m_Allocator.alloc<NodeMaybePredNah>();
        if(const auto scope = parseScope()) {
            nah->scope = scope.value();
        } else {
            std::cerr << "Expected scope" << std::endl;
            exit(EXIT_FAILURE);
        }

        const auto pred = m_Allocator.alloc<NodeMaybePred>();
        pred->var = nah;
        return pred;
    }

    return {};
}

std::optional<NodeStmt*> Parser::parseStmt() {
    if(peek().has_value() && peek().value().type == TokenType::bye && peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
        consume(); // bye
        consume(); // open paren
        NodeStmtBye* bye = m_Allocator.alloc<NodeStmtBye>();
        if(const auto expr = parseExpr()) {
            bye->expr = expr.value();
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        tryConsume(TokenType::close_paren, "Expected ')'");
        tryConsume(TokenType::semi, "Expected ';'");

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = bye;
        return stmt;
    }
    
    if (
        peek().has_value() && peek().value().type == TokenType::let 
        && peek(1).has_value() && peek(1).value().type == TokenType::ident 
        && peek(2).has_value() && peek(2).value().type == TokenType::eq
    ) {
        consume(); // let
        NodeStmtLet* let = m_Allocator.alloc<NodeStmtLet>();
        let->ident = consume();
        consume(); // eq

        if(const auto expr = parseExpr()) {
            let->expr = expr.value();
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        tryConsume(TokenType::semi, "Expected ';'");

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = let;
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
    
    if (tryConsume(TokenType::maybe)) {
        tryConsume(TokenType::open_paren, "Expected '('");
        NodeStmtMaybe* maybe = m_Allocator.alloc<NodeStmtMaybe>();
        if(const auto expr = parseExpr()) {
            maybe->expr = expr.value();
        } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        tryConsume(TokenType::close_paren, "Expected ')'");

        if(const auto scope = parseScope()) {
            maybe->scope = scope.value();
        } else {
            std::cerr << "Invalid scope" << std::endl;
            exit(EXIT_FAILURE);
        }

        maybe->pred = parseMaybePred();

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = maybe;
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