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
                tokens.push_back({ .type = TokenType::bye });
                buf.clear();
            } else if (buf == "let") {
                tokens.push_back({ .type = TokenType::let});
                buf.clear();
            } else {
                tokens.push_back({ .type = TokenType::ident, .value = buf });
                buf.clear();
            }
        } else if (std::isdigit(peek().value())) {
            buf.push_back(consume());
            while(peek().has_value() && std::isdigit(peek().value())) {
                buf.push_back(consume());
            }
            tokens.push_back({ .type = TokenType::int_lit, .value = buf });
            buf.clear();
        } else if (peek().value() == '(') {
            consume();
            tokens.push_back({ .type = TokenType::open_paren });
        } else if (peek().value() == ')') {
            consume();
            tokens.push_back({ .type = TokenType::close_paren });
        } else if (peek().value() == ';') {
            consume();
            tokens.push_back({ .type = TokenType::semi });
        } else if (peek().value() == '=') {
            consume();
            tokens.push_back({ .type = TokenType::eq });
        } else if (peek().value() == '+') {
            consume();
            tokens.push_back({ .type = TokenType::plus });
        } else if (peek().value() == '-') {
            consume();
            tokens.push_back({ .type = TokenType::minus });
        } else if (peek().value() == '*') {
            consume();
            tokens.push_back({ .type = TokenType::star });
        } else if (peek().value() == '/') {
            consume();
            tokens.push_back({ .type = TokenType::fslash });
        } else if(std::isspace(peek().value())) {
            consume();
        } else {
            std::cerr << "Invalid token" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    m_Index = 0;
    return tokens;
}

std::optional<char> Tokenizer::peek(int offset /*=0*/) const {
    if(m_Index + offset >= m_Src.size()) {
        return {};
    }

    return m_Src.at(m_Index + offset);
}

char Tokenizer::consume() {
    return m_Src.at(m_Index++);
}