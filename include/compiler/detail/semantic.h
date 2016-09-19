#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <detail/state.h>
#include <detail/ast.h>
#include <detail/ast_handler.h>

namespace avm {
typedef CompilerState::LevelInfo LevelInfo;
typedef CompilerState::LevelType LevelType;
typedef CompilerState::ModuleDefine ModuleDefine;
typedef CompilerState::ExternalFunction ExternalFunction;

class SemanticAnalyzer : public AstHandler {
public:
  SemanticAnalyzer(CompilerState *);

  void Analyze(AstModule *ast);
  void AddModule(const ModuleDefine &def);

protected:
  void Accept(AstModule *node);
  void Accept(AstNode *node);
  void Accept(AstImports *node);
  void Accept(AstImport *node);
  void Accept(AstStatement *node);
  void Accept(AstBlock *node);
  void Accept(AstExpression *node);
  void Accept(AstBinaryOp *node);
  void Accept(AstUnaryOp *node);
  void Accept(AstArrayAccess *node);
  void Accept(AstMemberAccess *node);
  void Accept(AstModuleAccess *node);
  void Accept(AstVariableDeclaration *node);
  void Accept(AstAlias *node);
  void Accept(AstUseModule *node);
  void Accept(AstVariable *node);
  void Accept(AstInteger *node);
  void Accept(AstFloat *node);
  void Accept(AstString *node);
  void Accept(AstTrue *node);
  void Accept(AstFalse *node);
  void Accept(AstNull *node);
  void Accept(AstSelf *node);
  void Accept(AstNew *node);
  void Accept(AstFunctionDefinition *node);
  void Accept(AstFunctionExpression *node);
  void Accept(AstFunctionCall *node);
  void Accept(AstClass *node);
  void Accept(AstObjectExpression *node);
  void Accept(AstEnum *node);
  void Accept(AstIfStmt *node);
  void Accept(AstPrintStmt *node);
  void Accept(AstReturnStmt *node);
  void Accept(AstForLoop *node);
  void Accept(AstWhileLoop *node);
  void Accept(AstTryCatch *node);
  void Accept(AstRange *node);

private:
  CompilerState *state_ptr;

  // Returns the use count of a variable/function.
  void IncrementUseCount(AstNode *);
  // Returns true if variable was found.
  bool FindVariable(const std::string &, bool = true);
  // Returns true if variable was found. A pointer to the variable is given.
  bool FindVariable(const std::string &, bool, Symbol *&);
  /* Returns true if variable was found, and if it is native,
  the number of arguments match.
  A pointer to the variable is given. */
  //bool FindVariable(const std::string &, size_t, bool, Symbol *&);

  void IncreaseBlock(LevelType);
  void DecreaseBlock();

  template <typename ... Args>
  void ErrorMsg(ErrorType type, SourceLocation loc, Args && ... args) {
    state_ptr->errors.push_back(BuildMessage(type, err_fatal, loc, args...));
  }

  template <typename ... Args>
  void WarningMessage(ErrorType type, SourceLocation loc, Args && ... args) {
    state_ptr->errors.push_back(BuildMessage(type, err_warning, loc, args...));
  }

  template <typename ... Args>
  void InfoMsg(ErrorType type, SourceLocation loc, Args && ... args) {
    state_ptr->errors.push_back(BuildMessage(type, err_info, loc, args...));
  }
};
} // namespace avm

#endif