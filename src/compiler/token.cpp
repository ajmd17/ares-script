#include <detail/token.h>

namespace avm {
std::map<TokenType, std::string> Token::token_strings = {
  { Token_unknown, "unknown" },
  { Token_identifier, "identifier" },
  { Token_string, "string" },
  { Token_integer, "integer" },
  { Token_float, "float" },
  { Token_keyword, "keyword" },
  { Token_operator, "operator" },
  { Token_open_parenthesis, "(" },
  { Token_close_parenthesis, ")" },
  { Token_open_brace, "{" },
  { Token_close_brace, "}" },
  { Token_open_bracket, "[" },
  { Token_close_bracket, "]" },
  { Token_semicolon, ";" },
  { Token_colon, ":" },
  { Token_comma, "," },
  { Token_period, "." },
  { Token_ellipsis, "..."},
  { Token_right_arrow, "->"},
  { Token_left_arrow, "<-"},
};

std::string Token::TokenTypeToString(TokenType type) {
  return token_strings.at(type);
}
} // namespace avm