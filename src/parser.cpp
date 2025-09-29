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
        if (tryConsume(TokenType::open_paren)) {
            // thingy call
            NodeTermCall* termCall = m_Allocator.alloc<NodeTermCall>();
            termCall->ident = ident.value();

            while(const auto expr = parseExpr()) {
                termCall->args.push_back(expr.value());
                if(!tryConsume(TokenType::comma)) {
                    break;
                }
            }

            tryConsumeErr(TokenType::close_paren);
            NodeTerm* term = m_Allocator.alloc<NodeTerm>();
            term->var = termCall;
            return term;
        }

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

    if(const auto _bool = tryConsume(TokenType::_bool)) {
        NodeTermBool* termBool = m_Allocator.alloc<NodeTermBool>();
        termBool->_bool = _bool.value();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termBool;
        return term;
    }

    if (const auto string = tryConsume(TokenType::string)) {
        NodeTermString* termString = m_Allocator.alloc<NodeTermString>();
        termString->string = string.value();

        NodeTerm* term = m_Allocator.alloc<NodeTerm>();
        term->var = termString;
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
        auto currToken = peek();
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

        // find binary operator
        auto it = tokenTypeToBinOp.find(op.type);
        if(it == tokenTypeToBinOp.end()) {
            errorExpected("binary operator");
        }

        NodeBinExpr* binExpr = m_Allocator.alloc<NodeBinExpr>();
        binExpr->op = it->second;
        binExpr->left = exprLeft;
        binExpr->right =  exprRight.value();

        NodeExpr* newExpr = m_Allocator.alloc<NodeExpr>();
        newExpr->var = binExpr;
        exprLeft = newExpr;
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
        consume(); // eq
        
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
        tryConsumeErr(TokenType::open_paren);
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

    if(tryConsume(TokenType::yell)) {
        tryConsumeErr(TokenType::open_paren);
        NodeStmtYell* yell = m_Allocator.alloc<NodeStmtYell>();
        if(const auto expr = parseExpr()) {
            yell->expr = expr.value();
        } else {
            errorExpected("expr");
        }
        tryConsumeErr(TokenType::close_paren);
        tryConsumeErr(TokenType::semi);

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = yell;
        return stmt;
    }

    if(tryConsume(TokenType::thingy)) {
        NodeStmtThingy* thingy = m_Allocator.alloc<NodeStmtThingy>();

        Token name = tryConsumeErr(TokenType::ident);
        thingy->name = name;

        tryConsumeErr(TokenType::open_paren);
        while(const auto ident = tryConsume(TokenType::ident)) {
            thingy->params.push_back(ident.value());
            if(!tryConsume(TokenType::comma).has_value()) {
                break;
            }
        }
        tryConsumeErr(TokenType::close_paren);

        if(const auto scope = parseScope()) {
            thingy->scope = scope.value();
        } else {
            errorExpected("scope");
        }

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = thingy;
        return stmt;
    }

    if(tryConsume(TokenType::gimmeback)) {
        NodeStmtGimmeback* gimmeback = m_Allocator.alloc<NodeStmtGimmeback>();

        if(auto expr = parseExpr()) {
            gimmeback->expr = expr.value();
        } else {
            errorExpected("expression");
        }

        tryConsumeErr(TokenType::semi);

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = gimmeback;
        return stmt;
    }

    if(tryConsume(TokenType::loop)) {
        NodeStmtLoop* loop = m_Allocator.alloc<NodeStmtLoop>();
        
        tryConsumeErr(TokenType::open_paren);

        Token ident = tryConsumeErr(TokenType::ident);
        loop->ident = ident;
        
        tryConsumeErr(TokenType::in);

        if(const auto start = parseExpr()) {
            loop->start = start.value();
        } else {
            errorExpected("expression");
        }

        tryConsumeErr(TokenType::dot);
        tryConsumeErr(TokenType::dot);

        if(const auto end = parseExpr()) {
            loop->end = end.value();
        } else {
            errorExpected("expression");
        }

        tryConsumeErr(TokenType::close_paren);

        if(const auto scope = parseScope()) {
            loop->scope = scope.value();
        } else {
            errorExpected("scope");
        }

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = loop;
        return stmt;
    }

    if(tryConsume(TokenType::why)) {
        NodeStmtWhy* why = m_Allocator.alloc<NodeStmtWhy>();

        tryConsumeErr(TokenType::open_paren);

        if (const auto expr = parseExpr()) {
            why->expr = expr.value();
        } else {
            errorExpected("expression");
        }

        tryConsumeErr(TokenType::close_paren);

        if (const auto scope = parseScope()) {
            why->scope = scope.value();
        } else {
            errorExpected("scope");
        }

        NodeStmt* stmt = m_Allocator.alloc<NodeStmt>();
        stmt->var = why;
        return stmt;
    }

    return {};
}

NodeProg Parser::parseProg() {
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
    const auto prev = peek(-1);
    const auto curr = peek(0);

    if (prev.has_value()) {
        const Token& token = prev.value();
        std::cerr << "[Parse Error] Expected " << msg << " at " << token.line << ":" << token.col<< std::endl;
    } else {
        const Token& token = curr.value();
        std::cerr << "[Parse Error] Expected " << msg << " before " << token.line << ":" << token.col << std::endl;
    }
    exit(EXIT_FAILURE);
}