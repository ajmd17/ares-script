#include <alexer.h>
#include <iostream>

#include <detail/syntax/keywords.h>
#include <detail/syntax/operators.h>

namespace avm {
Lexer::Lexer(const std::string &source, const std::string &filepath) {
  state.source = source;
  state.location.file = filepath;
  state.position = 0;
  state.length = source.length();
  state.location = SourceLocation(0, 0, filepath);
}

std::vector<Token> Lexer::ScanTokens() {
  std::vector<Token> tokens;
  SkipWhitespace();

  while (PeekChar() != '\0') {
    Token tok = NextToken();
    if (tok.type != TokenType::Token_unknown) {
      tokens.push_back(tok);
    }

    SkipWhitespace();
  }

  return tokens;
}

/** Reads the next token */
Token Lexer::NextToken() {
  SourceLocation loc = state.location;

  char ch = PeekChar();
  char next_char = PeekChar(1);
  char next_next_char = PeekChar(2);
  if (ch == '\'' || ch == '"') {
    return ReadStringLiteral();
  } else if (isdigit(ch) && (next_char == 'x' || next_char == 'X')) {
    return ReadHexNumberLiteral();
  } else if (isdigit(ch)) {
    return ReadNumberLiteral();
  } else if (ch == '.' && isdigit(next_char)) {
    // read float like .5
    std::string float_start = "0";
    return ReadFloatLiteral(float_start);
  } else if (ch == '_' || IsAlpha(ch)) {
    return ReadIdentifier();
  } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
    ch == '=' || ch == '!' || ch == '<' || ch == '>' ||
    ch == '&' || ch == '|' || ch == '^' ||
    ch == '?' || ch == '.' || ch == '#') {
    return ReadOperator();
  } else if (ch == '{') {
    ReadChar();
    return { TokenType::Token_open_brace, "{", loc };
  } else if (ch == '}') {
    ReadChar();
    return { TokenType::Token_close_brace, "}", loc };
  } else if (ch == '[') {
    ReadChar();
    return { TokenType::Token_open_bracket, "[", loc };
  } else if (ch == ']') {
    ReadChar();
    return { TokenType::Token_close_bracket, "]", loc };
  } else if (ch == '(') {
    ReadChar();
    return { TokenType::Token_open_parenthesis, "(", loc };
  } else if (ch == ')') {
    ReadChar();
    return { TokenType::Token_close_parenthesis, ")", loc };
  } else if (ch == ';') {
    ReadChar();
    return { TokenType::Token_semicolon, ";", loc };
  } else if (ch == ':') {
    ReadChar();
    return { TokenType::Token_colon, ":", loc };
  } else if (ch == ',') {
    ReadChar();
    return { TokenType::Token_comma, ",", loc };
  } else {
    ErrorMsg(ErrorType::msg_unexpected_token, loc, ReadChar());
    return {};
  }
}

Token Lexer::ReadNumberLiteral() {
  SourceLocation loc = state.location;
  std::string str;

  char ch = PeekChar();
  do {
    if (ch == '.') {
      return ReadFloatLiteral(str);
    }

    str += ReadChar();
    ch = PeekChar();
  } while (isdigit(ch) || ch == '.');

  return { TokenType::Token_integer, str, loc };
}

Token Lexer::ReadHexNumberLiteral() {
  SourceLocation loc = state.location;
  std::string str;

  str += ReadChar(); // read 0
  str += ReadChar(); // read x

  char ch = PeekChar();
  do {
    str += ReadChar();
    ch = PeekChar();
  } while (isxdigit(ch));

  long long value = std::stoll(str, 0, 16);
  std::stringstream ss;
  ss << value;

  return { TokenType::Token_integer, ss.str(), loc };
}

Token Lexer::ReadFloatLiteral(std::string &start) {
  SourceLocation loc = state.location;
  ReadChar();
  start += ".";

  char ch = PeekChar();
  do {
    start += ReadChar();
    ch = PeekChar();
  } while (isdigit(ch));

  return { TokenType::Token_float, start, loc };
}

Token Lexer::ReadStringLiteral() {
  SourceLocation loc = state.location;
  char delimiter = PeekChar();

  if (PeekChar(1) == delimiter && PeekChar(2) == delimiter) {
    return ReadMultistringLiteral();
  } else {
    ReadChar(); // eat the delimiter
    std::string str;

    char ch = PeekChar();
    while (ch != delimiter) {
      if (ch == '\\') {
        ReadChar();
        str += ParseEscape();
      } else if (ch == '\0' || ch == '\n') {
        break;
      } else {
        str += ReadChar();
      }
      ch = PeekChar();
    }

    if (ReadChar() != delimiter) {
      ErrorMsg(ErrorType::msg_unterminated_string_literal, loc);
    }

    return { TokenType::Token_string, str, loc };
  }
}

Token Lexer::ReadMultistringLiteral() {
  SourceLocation loc = state.location;
  char delimiter = ReadChar();
  ReadChar();
  ReadChar();

  std::string str;

  char ch = PeekChar();
  while (ch != delimiter) {
    if (ch == '\\') {
      ReadChar();
      str += ParseEscape();
    } else if (ch == '\0') {
      break;
    } else {
      str += ReadChar();
    }
    ch = PeekChar();
  }

  if (!((ReadChar() == delimiter) && (ReadChar() == delimiter) && (ReadChar() == delimiter))) {
    ErrorMsg(ErrorType::msg_unterminated_string_literal, loc);
  }

  return { TokenType::Token_string, str, loc };
}

bool Lexer::IsAlpha(char ch) const {
  return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
    ((unsigned char)ch >= 0xC0));
}

Token Lexer::ReadIdentifier() {
  SourceLocation loc = state.location;
  std::string str;

  char ch = PeekChar();
  do {
    str += ReadChar();
    ch = PeekChar();
  } while ((isdigit(ch) || IsAlpha(ch)) || ch == '_');

  Keyword kw = Keyword_FromString(str);
  if (kw == Keyword::Keyword_invalid) {
    return { TokenType::Token_identifier, str, loc };
  } else {
    return { TokenType::Token_keyword, str, loc };
  }
}

Token Lexer::ReadOperator() {
  SourceLocation loc = state.location;
  char ch = ReadChar();

  std::string next_two; // operators that consist of two chars
  next_two += ch;
  next_two += PeekChar();

  std::string next_three;
  next_three += ch;
  next_three += PeekChar(0);
  next_three += PeekChar(1);

  if (next_three == "...") {
    ReadChar();
    ReadChar();
    return { TokenType::Token_ellipsis, next_three, loc };
  }

  if (next_two == "->") {
    ReadChar();
    return { TokenType::Token_right_arrow, next_two, loc };
  } else if (next_two == "<-") {
    ReadChar();
    return { TokenType::Token_left_arrow, next_two, loc };
  }

  if (IsOperator(next_two)) {
    ReadChar();
    return { TokenType::Token_operator, next_two, loc };
  } else if (next_two == "/*") {
    ReadChar();
    ReadBlockComment();
    return { TokenType::Token_unknown, next_two, loc };
  }/* else if (next_two == "//") {
    ReadChar();
    ReadLineComment();
    return { TokenType::Token_unknown, next_two, loc };
  }*/

  std::string str;
  str += ch;
  switch (ch) {
  case '.':
    return { TokenType::Token_period, str, loc };
  case '#':
    ReadChar();
    ReadLineComment();
    return { TokenType::Token_unknown, str, loc };
  default:
    return { TokenType::Token_operator, str, loc };
  }
}

void Lexer::ReadLineComment() {
  char ch;
  do {
    ch = ReadChar();
  } while (ch != '\0' && ch != '\n');
}

void Lexer::ReadBlockComment() {
  while (PeekChar() != '\0') {
    char ch = ReadChar();
    if (ch == '*') {
      if (PeekChar() == '/') {
        ReadChar();
        return;
      }
    }
  }

  ErrorMsg(ErrorType::msg_unexpected_eof, state.location);
}

char Lexer::ReadChar() {
  if (state.position >= state.length) {
    return '\0';
  }

  if (state.source[state.position] == '\n') {
    ++state.location.line;
    state.location.column = 0;
  } else {
    ++state.location.column;
  }

  return state.source[state.position++];
}

char Lexer::PeekChar(int n) {
  if ((state.position + n) >= state.length) {
    return '\0';
  }

  return state.source[state.position + n];
}

char Lexer::ParseEscape() {
  SourceLocation loc = state.location;
  char ch = ReadChar();
  switch (ch) {
  case '"':
    return '"';
  case '\'':
    return '\'';
  case 'n':
    return '\n';
  case 'b':
    return '\b';
  case 'r':
    return '\r';
  case 'v':
    return '\v';
  case 't':
    return '\t';
  case 'f':
    return '\f';
  case '\\':
    return '\\';
  default:
    ErrorMsg(ErrorType::msg_unrecognized_escape_sequence, loc, ch);
    return '\0';
  }
}

void Lexer::SkipWhitespace() {
  while (isspace(PeekChar())) {
    ReadChar();
  }
}
} // namespace avm