#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <stack>

#include <detail/ast.h>
#include <detail/state.h>
#include <detail/location.h>
#include <detail/token.h>
#include <detail/error.h>

namespace avm {
class Parser {
public:
  ParserState state;

  Parser(std::vector<Token> tokens, LexerState lexerState);
  std::unique_ptr<AstModule> parse();

private:
  AstNode *main_module;

  std::string filepath;
  std::stack<std::string> variable_names;

  SourceLocation Location();

  Token *Peek(int n = 0);
  Token *Read();

  bool Match(TokenType, int n = 0);
  bool Match(TokenType, const std::string &);
  bool MatchRead(TokenType);
  bool MatchRead(TokenType, Token *&out);
  bool MatchRead(TokenType, const std::string &);
  Token *ExpectRead(TokenType);
  Token *ExpectRead(TokenType, const std::string &);

  int OpPrecedence();

  std::unique_ptr<AstNode> ParseImports();
  std::unique_ptr<AstNode> ParseImport();
  std::unique_ptr<AstNode> ParseStatement();
  std::unique_ptr<AstNode> ParseVariableDeclaration();
  std::unique_ptr<AstNode> ParseAlias();
  std::unique_ptr<AstNode> ParseUsing();
  std::unique_ptr<AstNode> ParseAttribute();
  std::unique_ptr<AstNode> ParseTerm();
  std::unique_ptr<AstNode> ParseExpression(bool pop_after = false);
  std::unique_ptr<AstNode> ParseParenthesis();
  std::unique_ptr<AstNode> ParseIntegerLiteral();
  std::unique_ptr<AstNode> ParseFloatLiteral();
  std::unique_ptr<AstNode> ParseIdentifier();
  std::unique_ptr<AstNode> ParseStringLiteral();
  std::unique_ptr<AstNode> ParseTrue();
  std::unique_ptr<AstNode> ParseFalse();
  std::unique_ptr<AstNode> ParseNull();
  std::unique_ptr<AstNode> ParseSelf();
  std::unique_ptr<AstNode> ParseNew(const std::string &);
  std::unique_ptr<AstNode> ParseBinaryOp(int prec, std::unique_ptr<AstNode> left);
  std::unique_ptr<AstNode> ParseUnaryOp();
  std::unique_ptr<AstNode> ParseClass();
  std::unique_ptr<AstNode> ParseEnum();
  std::unique_ptr<AstNode> ParseCodeBlock();
  std::unique_ptr<AstNode> ParseFunctionDefinition();
  std::unique_ptr<AstNode> ParseFunctionExpression();
  std::unique_ptr<AstNode> ParseIfStmt();
  std::unique_ptr<AstNode> ParsePrintStmt();
  std::unique_ptr<AstNode> ParseReturnStmt();
  std::unique_ptr<AstNode> ParseForLoop();
  std::unique_ptr<AstNode> ParseWhileLoop();
  std::unique_ptr<AstNode> ParseTryCatch();

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