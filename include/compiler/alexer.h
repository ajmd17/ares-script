#ifndef ALEXER_H
#define ALEXER_H

#include <vector>
#include <string>

#include <detail/state.h>

namespace avm {
class Lexer {
public:
  Lexer(const std::string &source, const std::string &filepath = "");

  LexerState state;
  std::vector<Token> ScanTokens();

private:
  Token NextToken();

  Token ReadNumberLiteral();
  Token ReadHexNumberLiteral();
  Token ReadFloatLiteral();
  Token ReadStringLiteral();
  Token ReadMultistringLiteral();
  Token ReadIdentifier();
  Token ReadOperator();

  void ReadLineComment();
  void ReadBlockComment();

  char ReadChar();
  char PeekChar(int n = 0);
  char ParseEscape();
  void SkipWhitespace();

  bool IsAlpha(char ch) const;

  template <typename ... Args>
  void ErrorMsg(ErrorType type, SourceLocation loc, Args && ... args) {
    state.errors.push_back(BuildMessage(type, err_fatal, loc, args...));
  }

  template <typename ... Args>
  void WarningMsg(ErrorType type, SourceLocation loc, Args && ... args) {
    state.errors.push_back(BuildMessage(type, err_warning, loc, args...));
  }

  template <typename ... Args>
  void InfoMsg(ErrorType type, SourceLocation loc, Args && ... args) {
    state.errors.push_back(BuildMessage(type, err_info, loc, args...));
  }
};
} // namespace avm

#endif