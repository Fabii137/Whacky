#pragma once

#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    let,
    ident,
    eq,

    plus,
    minus,
    star,
    fslash,

    open_paren,
    close_paren,
    open_curly,
    close_curly,


    bye, // exit()
    semi,

    int_lit, // int
    nothin, // void
    vibes, // bool

    thingy, // function
    gimmeback, // return

    maybe, // if
    nah, // else
    keepgoing, // while
    roundandround, // for

    yep, // true
    nope, // false

    nothingness, // null

    yell, // print
};

inline std::optional<int> binPrec(TokenType type) {
    switch(type) {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::star:
        case TokenType::fslash:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

class Tokenizer {
public:
    Tokenizer(std::string src);
    std::vector<Token> tokenize();

private:
    std::optional<char> peek(int offset = 0) const;
    char consume();
private:
    size_t m_Index = 0;
    const std::string m_Src;
};

