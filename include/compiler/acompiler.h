#ifndef ACOMPILER_H
#define ACOMPILER_H

#include <vector>
#include <string>
#include <map>

#include <detail/state.h>
#include <detail/ast.h>
#include <detail/ast_handler.h>
#include <common/bytecodes.h>

namespace avm {
typedef CompilerState::LevelInfo LevelInfo;
typedef CompilerState::Symbol Symbol;
typedef CompilerState::LevelType LevelType;
typedef CompilerState::ModuleDefine ModuleDefine;
typedef CompilerState::ExternalFunction ExternalFunction;

class Compiler : public AstHandler {
public:
  Compiler(const ParserState &);

  CompilerState &GetState();
  InstructionStream &GetInstructions();
  ModuleDefine &Module(const std::string &name);

  bool Compile(AstModule *node);

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
  CompilerState state;

  std::vector<ModuleDefine> native_modules;
  size_t UseCount(AstNode *);

  void IncreaseBlock(LevelType);
  void DecreaseBlock();

  InstructionStream bstream;
};
} // namespace avm

#endif