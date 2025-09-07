#include "Tokenizer.hpp"
#include <iostream>

Tokenizer::Tokenizer(std::string src): m_Src(std::move(src)) {

}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buf;

    while(peak().has_value()) {
        if(std::isalpha(peak().value())) {
            buf.push_back(consume());
            while(peak().has_value() && std::isalnum(peak().value())) {
                buf.push_back(consume());
            }
            if(buf == "bye") {
                tokens.push_back({ .type = TokenType::bye });
                buf.clear();
                continue;
            } else {
                std::cerr << "Invalid token" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (std::isdigit(peak().value())) {
            buf.push_back(consume());
            while(peak().has_value() && std::isdigit(peak().value())) {
                buf.push_back(consume());
            }
            tokens.push_back({ .type = TokenType::int_lit, .value = buf });
            buf.clear();
            continue;
        } else if (peak().value() == ';') {
            consume();
            tokens.push_back({ .type = TokenType::semi });
            continue;
        } else if(std::isspace(peak().value())) {
            consume();
            continue;
        } else {
            std::cerr << "Messed up" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    m_Index = 0;
    return tokens;
}

std::optional<char> Tokenizer::peak(int steps /*=1*/) const {
    if(m_Index + steps > m_Src.size()) {
        return {};
    }

    return m_Src.at(m_Index);
}

char Tokenizer::consume() {
    return m_Src.at(m_Index++);
}