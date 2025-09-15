#pragma once

#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    gimme, // let
    ident,
    eq,

    _or,
    _and,
    bor,
    band,
    _xor,
    neq,
    eqeq,
    ge,
    gt,
    le,
    lt,
    plus,
    minus,
    star,
    fslash,

    open_paren,
    close_paren,
    open_curly,
    close_curly,

    bye, // exit
    semi,

    int_lit,
    nothin, // void
    vibes, // bool

    thingy, // function
    gimmeback, // return

    maybe, // if
    but,// else if
    nah, // else

    keepgoing, // while
    roundandround, // for

    yep, // true
    nope, // false

    nothingness, // null

    yell, // print
};

inline std::string toString(const TokenType& type) {
    switch(type) {
        case TokenType::gimme: return "'gimme'";
        case TokenType::ident: return "identifier";
        case TokenType::eq: return "'='";
        case TokenType::_or: return "'or'";
        case TokenType::_and: return "'and'";
        case TokenType::bor: return "'lor'";
        case TokenType::band: return "'land'";
        case TokenType::_xor: return "'xor'";
        case TokenType::neq: return "'!='";
        case TokenType::eqeq: return "'=='";
        case TokenType::ge: return "'>='";
        case TokenType::gt: return "'>'";
        case TokenType::le: return "'<='";
        case TokenType::lt: return "'<'";
        case TokenType::plus: return "'+'";
        case TokenType::minus: return "'-'";
        case TokenType::star: return "'*'";
        case TokenType::fslash: return "'/'";
        case TokenType::open_paren: return "'('";
        case TokenType::close_paren: return "')'";
        case TokenType::open_curly: return "'{'";
        case TokenType::close_curly: return "'}'";
        case TokenType::bye: return "'bye'";
        case TokenType::semi: return "';'";
        case TokenType::int_lit: return "int literal";
        case TokenType::nothin: return "'nothin'";
        case TokenType::vibes: return "'vibes'";
        case TokenType::thingy: return "'thingy'";
        case TokenType::gimmeback: return "'gimmeback'";
        case TokenType::maybe: return "'maybe'";
        case TokenType::but: return "'but'";
        case TokenType::nah: return "'nah'";
        case TokenType::keepgoing: return "'keepgoing'";
        case TokenType::roundandround: return "'roundandround'";
        case TokenType::yep: return "'yep'";
        case TokenType::nope: return "'nope'";
        case TokenType::nothingness: return "'nothingness'";
        case TokenType::yell: return "'yell'";
        default: return "unknown";
    }
}

inline std::optional<int> binPrec(const TokenType type) {
    switch(type) {
        case TokenType::_or:
        case TokenType::_and:
            return 0;
        case TokenType::bor:
        case TokenType::band:
        case TokenType::_xor:
            return 1;
        case TokenType::neq:
        case TokenType::eqeq:
        case TokenType::ge:
        case TokenType::gt:
        case TokenType::le:
        case TokenType::lt:
            return 2;
        case TokenType::plus:
        case TokenType::minus:
            return 3;
        case TokenType::star:
        case TokenType::fslash:
            return 4;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    size_t line;
    size_t col;
    std::optional<std::string> value;
};

class Tokenizer {
public:
    Tokenizer(std::string src);
    std::vector<Token> tokenize();

private:
    std::optional<char> peek(size_t offset = 0) const;
    char consume();
private:
    size_t m_Index = 0;
    size_t m_Line = 1;
    size_t m_Col = 1;
    const std::string m_Src;
};

