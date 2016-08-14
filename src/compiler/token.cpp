#include <detail/token.h>

namespace avm {
std::map<TokenType, std::string> Token::token_strings = {
  { tk_unknown, "unknown" },
  { tk_identifier, "identifier" },
  { tk_string, "string" },
  { tk_integer, "integer" },
  { tk_float, "float" },
  { tk_keyword, "keyword" },
  { tk_operator, "operator" },
  { tk_open_parenthesis, "(" },
  { tk_close_parenthesis, ")" },
  { tk_open_brace, "{" },
  { tk_close_brace, "}" },
  { tk_open_bracket, "[" },
  { tk_close_bracket, "]" },
  { tk_semicolon, ";" },
  { tk_colon, ":" },
  { tk_comma, "," },
  { tk_period, "." },
  { tk_ellipsis, "..."},
  { tk_right_arrow, "->"},
  { tk_left_arrow, "<-"},
};

std::string Token::TokenTypeToString(TokenType type) {
  return token_strings.at(type);
}
} // namespace avm