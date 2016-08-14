#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <map>

#include <detail/location.h>

namespace avm {
enum TokenType {
  tk_unknown = -1,
  tk_identifier,
  tk_string,
  tk_integer,
  tk_float,
  tk_keyword,
  tk_operator,
  tk_open_parenthesis,
  tk_close_parenthesis,
  tk_open_brace,
  tk_close_brace,
  tk_open_bracket,
  tk_close_bracket,
  tk_semicolon,
  tk_colon,
  tk_comma,
  tk_period,
  tk_ellipsis,
  tk_right_arrow,
  tk_left_arrow,
};

struct Token {
public:
  TokenType type;
  std::string value;
  SourceLocation location;

  static std::string TokenTypeToString(TokenType type);

private:
  static std::map<TokenType, std::string> token_strings;
};
} // namespace avm

#endif