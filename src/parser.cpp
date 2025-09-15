#include "Parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_Tokens(std::move(tokens)), m_Allocator(1024 * 1024 * 4) /* 4 mb */ {

}

std::optional<NodeTerm*> Parser::parseTerm() {
    if(const auto intLit = tryConsume(TokenType::int_lit)) {
        NodeTermIntLit* termIntLit = m_Allocator.alloc<NodeTermIntLit>();
        termIntLit->int_lit = intLit.value();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termIntLit;
        return term;
    } 

    if (const auto ident = tryConsume(TokenType::ident)) {
        NodeTermIdent* termIdent = m_Allocator.alloc<NodeTermIdent>();
        termIdent->ident = ident.value();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termIdent;
        return term;
    }

    if (const auto openParen = tryConsume(TokenType::open_paren)) {
        auto expr = parseExpr();
        if(!expr.has_value()) {
            errorExpected("expression");
        }

        tryConsumeErr(TokenType::close_paren);
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

        const Token& op = consume();
        const int nextMinPrec = prec.value() + 1;
        auto exprRight = parseExpr(nextMinPrec);
        if(!exprRight.has_value()) {
            errorExpected("expression");
        }

        auto binExpr = m_Allocator.alloc<NodeBinExpr>();
        auto exprLeft2 = m_Allocator.alloc<NodeExpr>();

        switch(op.type) {
            case TokenType::_or: {
                NodeBinExprOr* _or = m_Allocator.alloc<NodeBinExprOr>();
                exprLeft2->var = exprLeft->var;
                _or->left = exprLeft2;
                _or->right = exprRight.value();
                binExpr->var = _or;
                break;
            }
            case TokenType::_and: {
                NodeBinExprAnd* _and = m_Allocator.alloc<NodeBinExprAnd>();
                exprLeft2->var = exprLeft->var;
                _and->left = exprLeft2;
                _and->right = exprRight.value();
                binExpr->var = _and;
                break;
            }
            case TokenType::bor: {
                NodeBinExprBor* bor = m_Allocator.alloc<NodeBinExprBor>();
                exprLeft2->var = exprLeft->var;
                bor->left = exprLeft2;
                bor->right = exprRight.value();
                binExpr->var = bor;
                break;
            }
            case TokenType::band: {
                NodeBinExprBand* band = m_Allocator.alloc<NodeBinExprBand>();
                exprLeft2->var = exprLeft->var;
                band->left = exprLeft2;
                band->right = exprRight.value();
                binExpr->var = band;
                break;
            }
            case TokenType::_xor: {
                NodeBinExprXor* _xor = m_Allocator.alloc<NodeBinExprXor>();
                exprLeft2->var = exprLeft->var;
                _xor->left = exprLeft2;
                _xor->right = exprRight.value();
                binExpr->var = _xor;
                break;
            }
            case TokenType::neq: {
                NodeBinExprNeq* neq = m_Allocator.alloc<NodeBinExprNeq>();
                exprLeft2->var = exprLeft->var;
                neq->left = exprLeft2;
                neq->right = exprRight.value();
                binExpr->var = neq;
                break;
            }
            case TokenType::eqeq: {
                NodeBinExprEq* eq = m_Allocator.alloc<NodeBinExprEq>();
                exprLeft2->var = exprLeft->var;
                eq->left = exprLeft2;
                eq->right = exprRight.value();
                binExpr->var = eq;
                break;
            }
            case TokenType::ge: {
                NodeBinExprGe* ge = m_Allocator.alloc<NodeBinExprGe>();
                exprLeft2->var = exprLeft->var;
                ge->left = exprLeft2;
                ge->right = exprRight.value();
                binExpr->var = ge;
                break;
            }
            case TokenType::gt: {
                NodeBinExprGt* gt = m_Allocator.alloc<NodeBinExprGt>();
                exprLeft2->var = exprLeft->var;
                gt->left = exprLeft2;
                gt->right = exprRight.value();
                binExpr->var = gt;
                break;
            }
            case TokenType::le: {
                NodeBinExprLe* le = m_Allocator.alloc<NodeBinExprLe>();
                exprLeft2->var = exprLeft->var;
                le->left = exprLeft2;
                le->right = exprRight.value();
                binExpr->var = le;
                break;
            }
            case TokenType::lt: {
                NodeBinExprLt* lt = m_Allocator.alloc<NodeBinExprLt>();
                exprLeft2->var = exprLeft->var;
                lt->left = exprLeft2;
                lt->right = exprRight.value();
                binExpr->var = lt;
                break;
            }
            case TokenType::plus: {
                NodeBinExprAdd* add = m_Allocator.alloc<NodeBinExprAdd>();
                exprLeft2->var = exprLeft->var;
                add->left = exprLeft2;
                add->right = exprRight.value();
                binExpr->var = add;
                break;
            }
            case TokenType::minus: {
                NodeBinExprSub* sub = m_Allocator.alloc<NodeBinExprSub>();
                exprLeft2->var = exprLeft->var;
                sub->left = exprLeft2;
                sub->right = exprRight.value();
                binExpr->var = sub;
                break;
            }
            case TokenType::star: {
                NodeBinExprMul* mul = m_Allocator.alloc<NodeBinExprMul>();
                exprLeft2->var = exprLeft->var;
                mul->left = exprLeft2;
                mul->right = exprRight.value();
                binExpr->var = mul;
                break;
            }
            case TokenType::fslash: {
                NodeBinExprDiv* div = m_Allocator.alloc<NodeBinExprDiv>();
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

    tryConsumeErr(TokenType::close_curly);

    return scope;
}

std::optional<NodeMaybePred*> Parser::parseMaybePred() {
    if(tryConsume(TokenType::but)) {
        tryConsumeErr(TokenType::open_paren);
        const auto but = m_Allocator.alloc<NodeMaybePredBut>();
        if(const auto expr = parseExpr()) {
            but->expr = expr.value();
        } else {
            errorExpected("expression");
        }

        tryConsumeErr(TokenType::close_paren);

        if(const auto scope = parseScope()) {
            but->scope = scope.value();
        
        } else {
            errorExpected("scope");
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
            errorExpected("scope");
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
            errorExpected("expression");
        }

        tryConsumeErr(TokenType::close_paren);
        tryConsumeErr(TokenType::semi);

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = bye;
        return stmt;
    }
    
    if (
        peek().has_value() && peek().value().type == TokenType::gimme 
        && peek(1).has_value() && peek(1).value().type == TokenType::ident 
        && peek(2).has_value() && peek(2).value().type == TokenType::eq
    ) {
        consume(); // gimme
        NodeStmtGimme* gimme = m_Allocator.alloc<NodeStmtGimme>();
        gimme->ident = consume();
        consume(); // eq

        if(const auto expr = parseExpr()) {
            gimme->expr = expr.value();
        } else {
            errorExpected("expression");
        }
        tryConsumeErr(TokenType::semi);

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = gimme;
        return stmt;
    }

    if (peek().has_value() && peek().value().type == TokenType::ident
        && peek(1).has_value() && peek(1).value().type == TokenType::eq
    ) {
        NodeStmtAssignment* assignment = m_Allocator.alloc<NodeStmtAssignment>();
        assignment->ident = consume();
        consume(); // =
        
        if(const auto expr = parseExpr()) {
            assignment->expr = expr.value();
        } else {
            errorExpected("expression");
        }
        tryConsumeErr(TokenType::semi);

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = assignment;
        return stmt;
    }
    
    if (peek().has_value() && peek().value().type == TokenType::open_curly) {
        if(const auto scope = parseScope()) {
            NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
            stmt->var = scope.value();
            return stmt;
        }

        errorExpected("scope");
    }
    
    if (tryConsume(TokenType::maybe)) {
        tryConsume(TokenType::open_paren);
        NodeStmtMaybe* maybe = m_Allocator.alloc<NodeStmtMaybe>();
        if(const auto expr = parseExpr()) {
            maybe->expr = expr.value();
        } else {
            errorExpected("expression");
        }
        
        tryConsumeErr(TokenType::close_paren);

        if(const auto scope = parseScope()) {
            maybe->scope = scope.value();
        } else {
            errorExpected("scope");
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
        if (const auto stmt = parseStmt()) {
            prog.stmts.push_back(stmt.value());
        } else {
            errorExpected("statement");
        }
    }
    return prog;
}


std::optional<Token> Parser::peek(const int offset /*=0*/) const {
    if(m_Index + offset >= m_Tokens.size()) {
        return {};
    }

    return m_Tokens.at(m_Index + offset);
}

Token Parser::consume() {
    return m_Tokens.at(m_Index++);
}

Token Parser::tryConsumeErr(const TokenType& type) {
    if(peek().has_value() && peek().value().type == type) {
        return consume();
    }

    errorExpected(toString(type));
    return {};
}

std::optional<Token> Parser::tryConsume(const TokenType& type) {
    if(peek().has_value() && peek().value().type == type) {
        return consume();
    }

    return {};
}

void Parser::errorExpected(const std::string& msg) const {
    auto prev = peek(-1);
    auto curr = peek(0);

    if (prev.has_value()) {
        const Token& token = prev.value();
        std::cerr << "[Parse Error] Expected " << msg << " at " << token.line << ":" << token.col<< std::endl;
    } else {
        const Token& token = curr.value();
        std::cerr << "[Parse Error] Expected " << msg << " before " << token.line << ":" << token.col << std::endl;
    }
    exit(EXIT_FAILURE);
}