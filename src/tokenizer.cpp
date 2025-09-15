#include "Tokenizer.hpp"
#include <iostream>

Tokenizer::Tokenizer(std::string src): m_Src(std::move(src)) {

}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buf;

    while(peek().has_value()) {
        if(std::isalpha(peek().value())) {
            buf.push_back(consume());
            while(peek().has_value() && std::isalnum(peek().value())) {
                buf.push_back(consume());
            }

            if(buf == "bye") {
                tokens.push_back({ TokenType::bye, m_Line, m_Col });
            } else if (buf == "gimme") {
                tokens.push_back({ TokenType::gimme, m_Line, m_Col });
            } else if (buf == "yep") {
                tokens.push_back({ TokenType::_bool, m_Line, m_Col, "1" });
            } else if (buf == "nope") {
                tokens.push_back({ TokenType::_bool, m_Line, m_Col, "0" });
            } else if (buf == "maybe") {
                tokens.push_back({ TokenType::maybe, m_Line, m_Col });
            } else if (buf == "but") {
                tokens.push_back({ TokenType::but, m_Line, m_Col });
            } else if (buf == "nah") {
                tokens.push_back({ TokenType::nah, m_Line, m_Col });
            } else if (buf == "and") {
                tokens.push_back({ TokenType::_and, m_Line, m_Col });
            } else if (buf == "or") {
                tokens.push_back({ TokenType::_or, m_Line, m_Col });
            } else if (buf == "band") {
                tokens.push_back({ TokenType::band, m_Line, m_Col });
            } else if (buf == "bor") {
                tokens.push_back({ TokenType::bor, m_Line, m_Col });
            } else if (buf == "xor") {
                tokens.push_back({ TokenType::_xor, m_Line, m_Col });
            } else {
                tokens.push_back({ TokenType::ident, m_Line, m_Col, buf });
            }
            buf.clear();
        } else if (std::isdigit(peek().value())) {
            buf.push_back(consume());
            while(peek().has_value() && std::isdigit(peek().value())) {
                buf.push_back(consume());
            }
            tokens.push_back({ TokenType::int_lit, m_Line, m_Col, buf });
            buf.clear();
        } else if(peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
            consume();
            consume();
            while (peek().has_value() && peek().value() != '\n') {
                consume();
            }
        } else if(peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
            consume();
            consume();
            while (peek().has_value()) {
                if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                    consume();
                    consume();
                    break;
                }
                consume();
            }
        } else if (peek().value() == '(') {
            consume();
            tokens.push_back({ TokenType::open_paren, m_Line, m_Col });
        } else if (peek().value() == ')') {
            consume();
            tokens.push_back({ TokenType::close_paren, m_Line, m_Col });
        } else if (peek().value() == ';') {
            consume();
            tokens.push_back({ TokenType::semi, m_Line, m_Col });
        } else if (peek().value() == '=') {
            if (peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({ TokenType::eqeq, m_Line, m_Col });
                continue;
            }
            consume();
            tokens.push_back({ TokenType::eq, m_Line, m_Col });
        } else if (peek().value() == '!') {
            if(peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({ TokenType::neq, m_Line, m_Col });
                continue;
            }

            std::cerr << "Invalid token(!)" << std::endl;
            exit(EXIT_FAILURE);
        } else if (peek().value() == '>') {
            if (peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({ TokenType::ge, m_Line, m_Col });
                continue;
            }
            consume();
            tokens.push_back({ TokenType::gt, m_Line, m_Col });
        } else if (peek().value() == '<') {
            if (peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({ TokenType::le, m_Line, m_Col });
                continue;
            }
            consume();
            tokens.push_back({ TokenType::lt, m_Line, m_Col });
        } else if (peek().value() == '+') {
            consume();
            tokens.push_back({ TokenType::plus, m_Line, m_Col });
        } else if (peek().value() == '-') {
            consume();
            tokens.push_back({ TokenType::minus, m_Line, m_Col });
        } else if (peek().value() == '*') {
            consume();
            tokens.push_back({ TokenType::star, m_Line, m_Col });
        } else if (peek().value() == '/') {
            consume();
            tokens.push_back({ TokenType::fslash, m_Line, m_Col });
        } else if (peek().value() == '{') {
            consume();
            tokens.push_back({ TokenType::open_curly, m_Line, m_Col });
        } else if (peek().value() == '}') {
            consume();
            tokens.push_back({ TokenType::close_curly, m_Line, m_Col });
        } else if (peek().value() == '\n') {
            consume();
        } else if(std::isspace(peek().value())) {
            consume();
        } else {
            std::cerr << "[Tokenize Error] Invalid token at " << m_Line << ":" << m_Col << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    m_Index = 0;
    return tokens;
}

std::optional<char> Tokenizer::peek(size_t offset /*=0*/) const {
    if(m_Index + offset >= m_Src.size()) {
        return {};
    }

    return m_Src.at(m_Index + offset);
}

char Tokenizer::consume() {
    char c =  m_Src.at(m_Index++);
    if(c == '\n') {
        m_Line++;
        m_Col = 1;
    } else {
        m_Col++;
    }
    return c;
}