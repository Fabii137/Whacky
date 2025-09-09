#pragma once

#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    let,
    ident,
    eq,

    plus,
    mul,

    open_paren,
    close_paren,


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

