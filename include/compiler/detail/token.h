#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <map>

#include <detail/location.h>

namespace avm {
enum TokenType {
    Token_unknown = -1,
    Token_identifier,
    Token_string,
    Token_integer,
    Token_float,
    Token_keyword,
    Token_operator,
    Token_open_parenthesis,
    Token_close_parenthesis,
    Token_open_brace,
    Token_close_brace,
    Token_open_bracket,
    Token_close_bracket,
    Token_semicolon,
    Token_colon,
    Token_comma,
    Token_period,
    Token_ellipsis,
    Token_right_arrow,
    Token_left_arrow,
};

struct Token {
public:
    TokenType type;
    std::string value;
    SourceLocation location;

    static inline std::string TokenTypeToString(TokenType type) { return token_strings.at(type); }

private:
    static std::map<TokenType, std::string> token_strings;
};
} // namespace avm

#endif