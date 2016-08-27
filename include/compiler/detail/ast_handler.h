#ifndef AST_HANDLER_H
#define AST_HANDLER_H

#include <memory>

#include <detail/ast.h>

namespace avm {
class AstHandler {
public:
  virtual void Accept(AstModule *node) = 0;

protected:
  virtual void Accept(AstNode *node) = 0;
  virtual void Accept(AstImports *node) = 0;
  virtual void Accept(AstImport *node) = 0;
  virtual void Accept(AstStatement *node) = 0;
  virtual void Accept(AstBlock *node) = 0;
  virtual void Accept(AstExpression *node) = 0;
  virtual void Accept(AstBinaryOp *node) = 0;
  virtual void Accept(AstUnaryOp *node) = 0;
  virtual void Accept(AstArrayAccess *node) = 0;
  virtual void Accept(AstMemberAccess *node) = 0;
  virtual void Accept(AstModuleAccess *node) = 0;
  virtual void Accept(AstVariableDeclaration *node) = 0;
  virtual void Accept(AstAlias *node) = 0;
  virtual void Accept(AstUseModule *node) = 0;
  virtual void Accept(AstVariable *node) = 0;
  virtual void Accept(AstInteger *node) = 0;
  virtual void Accept(AstFloat *node) = 0;
  virtual void Accept(AstString *node) = 0;
  virtual void Accept(AstTrue *node) = 0;
  virtual void Accept(AstFalse *node) = 0;
  virtual void Accept(AstNull *node) = 0;
  virtual void Accept(AstSelf *node) = 0;
  virtual void Accept(AstNew *node) = 0;
  virtual void Accept(AstFunctionDefinition *node) = 0;
  virtual void Accept(AstFunctionExpression *node) = 0;
  virtual void Accept(AstFunctionCall *node) = 0;
  virtual void Accept(AstClass *node) = 0;
  virtual void Accept(AstObjectExpression *node) = 0;
  virtual void Accept(AstEnum *node) = 0;
  virtual void Accept(AstIfStmt *node) = 0;
  virtual void Accept(AstPrintStmt *node) = 0;
  virtual void Accept(AstReturnStmt *node) = 0;
  virtual void Accept(AstForLoop *node) = 0;
  virtual void Accept(AstWhileLoop *node) = 0;
  virtual void Accept(AstTryCatch *node) = 0;
  virtual void Accept(AstRange *node) = 0;

  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstNode> node) {
    if (!node) {
      return nullptr;
    }

    switch (node->type) {
    case ast_type_imports:
      return optimize(std::move(std::unique_ptr<AstImports>(static_cast<AstImports*>(node.release()))));
    case ast_type_import:
      return optimize(std::move(std::unique_ptr<AstImport>(static_cast<AstImport*>(node.release()))));
    case ast_type_statement:
      return optimize(std::move(std::unique_ptr<AstStatement>(static_cast<AstStatement*>(node.release()))));
    case ast_type_block:
      return optimize(std::move(std::unique_ptr<AstBlock>(static_cast<AstBlock*>(node.release()))));
    case ast_type_expression:
      return optimize(std::move(std::unique_ptr<AstExpression>(static_cast<AstExpression*>(node.release()))));
    case ast_type_binop:
      return optimize(std::move(std::unique_ptr<AstBinaryOp>(static_cast<AstBinaryOp*>(node.release()))));
    case ast_type_unop:
      return optimize(std::move(std::unique_ptr<AstUnaryOp>(static_cast<AstUnaryOp*>(node.release()))));
    case ast_type_array_access:
      return optimize(std::move(std::unique_ptr<AstArrayAccess>(static_cast<AstArrayAccess*>(node.release()))));
    case ast_type_member_access:
      return optimize(std::move(std::unique_ptr<AstMemberAccess>(static_cast<AstMemberAccess*>(node.release()))));
    case ast_type_module_access:
      return optimize(std::move(std::unique_ptr<AstModuleAccess>(static_cast<AstModuleAccess*>(node.release()))));
    case ast_type_var_declaration:
      return optimize(std::move(std::unique_ptr<AstVariableDeclaration>(static_cast<AstVariableDeclaration*>(node.release()))));
    case ast_type_alias:
      return optimize(std::move(std::unique_ptr<AstAlias>(static_cast<AstAlias*>(node.release()))));
    case ast_type_use_module:
      return optimize(std::move(std::unique_ptr<AstUseModule>(static_cast<AstUseModule*>(node.release()))));
    case ast_type_variable:
      return optimize(std::move(std::unique_ptr<AstVariable>(static_cast<AstVariable*>(node.release()))));
    case ast_type_integer:
      return optimize(std::move(std::unique_ptr<AstInteger>(static_cast<AstInteger*>(node.release()))));
    case ast_type_float:
      return optimize(std::move(std::unique_ptr<AstFloat>(static_cast<AstFloat*>(node.release()))));
    case ast_type_string:
      return optimize(std::move(std::unique_ptr<AstString>(static_cast<AstString*>(node.release()))));
    case ast_type_true:
      return optimize(std::move(std::unique_ptr<AstTrue>(static_cast<AstTrue*>(node.release()))));
    case ast_type_false:
      return optimize(std::move(std::unique_ptr<AstFalse>(static_cast<AstFalse*>(node.release()))));
    case ast_type_null:
      return optimize(std::move(std::unique_ptr<AstNull>(static_cast<AstNull*>(node.release()))));
    case ast_type_self:
      return optimize(std::move(std::unique_ptr<AstSelf>(static_cast<AstSelf*>(node.release()))));
    case ast_type_new:
      return optimize(std::move(std::unique_ptr<AstNew>(static_cast<AstNew*>(node.release()))));
    case ast_type_function_definition:
      return optimize(std::move(std::unique_ptr<AstFunctionDefinition>(static_cast<AstFunctionDefinition*>(node.release()))));
    case ast_type_function_expression:
      return optimize(std::move(std::unique_ptr<AstFunctionExpression>(static_cast<AstFunctionExpression*>(node.release()))));
    case ast_type_function_call:
      return optimize(std::move(std::unique_ptr<AstFunctionCall>(static_cast<AstFunctionCall*>(node.release()))));
    case ast_type_class_declaration:
      return optimize(std::move(std::unique_ptr<AstClass>(static_cast<AstClass*>(node.release()))));
    case ast_type_object_expression:
      return optimize(std::move(std::unique_ptr<AstObjectExpression>(static_cast<AstObjectExpression*>(node.release()))));
    case ast_type_print:
      return optimize(std::move(std::unique_ptr<AstPrintStmt>(static_cast<AstPrintStmt*>(node.release()))));
    case ast_type_return:
      return optimize(std::move(std::unique_ptr<AstReturnStmt>(static_cast<AstReturnStmt*>(node.release()))));
    case ast_type_if_statement:
      return optimize(std::move(std::unique_ptr<AstIfStmt>(static_cast<AstIfStmt*>(node.release()))));
    case ast_type_for_loop:
      return optimize(std::move(std::unique_ptr<AstForLoop>(static_cast<AstForLoop*>(node.release()))));
    case ast_type_while_loop:
      return optimize(std::move(std::unique_ptr<AstWhileLoop>(static_cast<AstWhileLoop*>(node.release()))));
    case ast_type_try_catch:
      return optimize(std::move(std::unique_ptr<AstTryCatch>(static_cast<AstTryCatch*>(node.release()))));
    case ast_type_range:
      return optimize(std::move(std::unique_ptr<AstRange>(static_cast<AstRange*>(node.release()))));
    default:
      return nullptr;
    }
  }

  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstImports> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstImport> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstStatement> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstBlock> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstExpression> node) { return std::move(node); }

  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstBinaryOp> node) {
    node->left = optimize(std::move(node->left));
    node->right = optimize(std::move(node->right));

    if (node->left->type == ast_type_integer && node->right->type == ast_type_integer)
      return std::move(optimize_bin_op<AstInteger, AstInteger, AstInteger>(node->left.get(), node->right.get(), node->op));
    else if (node->left->type == ast_type_integer && node->right->type == ast_type_float)
      return std::move(optimize_bin_op<AstFloat, AstInteger, AstFloat>(node->left.get(), node->right.get(), node->op));
    else if (node->left->type == ast_type_float && node->right->type == ast_type_integer)
      return std::move(optimize_bin_op<AstFloat, AstFloat, AstInteger>(node->left.get(), node->right.get(), node->op));
    else if (node->left->type == ast_type_float && node->right->type == ast_type_float)
      return std::move(optimize_bin_op<AstFloat, AstFloat, AstFloat>(node->left.get(), node->right.get(), node->op));
    else
      return std::move(node);
  }

  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstUnaryOp> node) {
    node->child = optimize(std::move(node->child));

    if (node->child->type == ast_type_integer)
      return std::move(optimize_un_op<AstInteger, AstInteger>(node->child.get(), node->op));
    else if (node->child->type == ast_type_float)
      return std::move(optimize_un_op<AstFloat, AstFloat>(node->child.get(), node->op));
    else
      return std::move(node);
  }

  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstArrayAccess> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstMemberAccess> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstModuleAccess> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstVariableDeclaration> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstAlias> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstUseModule> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstVariable> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstInteger> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstFloat> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstString> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstTrue> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstFalse> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstNull> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstSelf> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstNew> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstFunctionDefinition> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstFunctionExpression> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstFunctionCall> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstClass> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstObjectExpression> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstEnum> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstIfStmt> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstPrintStmt> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstReturnStmt> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstForLoop> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstWhileLoop> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstTryCatch> node) { return std::move(node); }
  virtual std::unique_ptr<AstNode> optimize(std::unique_ptr<AstRange> node) { return std::move(node); }

private:
  template <typename ReturnTypeAst, typename T1, typename T2>
  std::unique_ptr<AstNode> optimize_bin_op(AstNode *a, AstNode *b, BinaryOp op) {
    typedef decltype(ReturnTypeAst::value) ReturnValueType;

    T1 *left = dynamic_cast<T1*>(a);
    T2 *right = dynamic_cast<T2*>(b);

    switch (op) {
    case BinOp_power:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, pow(static_cast<ReturnValueType>(left->value), static_cast<ReturnValueType>(right->value)))));
    case BinOp_multiply:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, static_cast<ReturnValueType>(left->value) * static_cast<ReturnValueType>(right->value))));
    case BinOp_floor_divide:
    case BinOp_divide:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, static_cast<ReturnValueType>(left->value) / static_cast<ReturnValueType>(right->value))));
    case BinOp_modulus:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<AVMInteger_t>(left->value) % static_cast<AVMInteger_t>(right->value))));
    case BinOp_add:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, static_cast<ReturnValueType>(left->value) + static_cast<ReturnValueType>(right->value))));
    case BinOp_subtract:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, static_cast<ReturnValueType>(left->value) - static_cast<ReturnValueType>(right->value))));
    case BinOp_logand:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) && static_cast<ReturnValueType>(right->value))));
    case BinOp_logor:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) || static_cast<ReturnValueType>(right->value))));
    case BinOp_equals:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) == static_cast<ReturnValueType>(right->value))));
    case BinOp_not_equal:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) != static_cast<ReturnValueType>(right->value))));
    case BinOp_less:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) < static_cast<ReturnValueType>(right->value))));
    case BinOp_greater:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) > static_cast<ReturnValueType>(right->value))));
    case BinOp_less_eql:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) <= static_cast<ReturnValueType>(right->value))));
    case BinOp_greater_eql:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<ReturnValueType>(left->value) >= static_cast<ReturnValueType>(right->value))));
    case BinOp_bitand:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<AVMInteger_t>(left->value) & static_cast<AVMInteger_t>(right->value))));
    case BinOp_bitor:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<AVMInteger_t>(left->value) | static_cast<AVMInteger_t>(right->value))));
    case BinOp_bitxor:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<AVMInteger_t>(left->value) ^ static_cast<AVMInteger_t>(right->value))));
    case BinOp_bitshift_left:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<AVMInteger_t>(left->value) << static_cast<AVMInteger_t>(right->value))));
    case BinOp_bitshift_right:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, static_cast<AVMInteger_t>(left->value) >> static_cast<AVMInteger_t>(right->value))));
    default:
      return nullptr;
    }
  }

  template <typename ReturnTypeAst, typename T1>
  std::unique_ptr<AstNode> optimize_un_op(AstNode *a, UnaryOp op) {
    typedef decltype(ReturnTypeAst::value) ReturnValueType;

    T1 *left = dynamic_cast<T1*>(a);
    switch (op) {
    case UnOp_lognot:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, !static_cast<ReturnValueType>(left->value))));
    case UnOp_bitcompl:
      return std::move(std::unique_ptr<AstInteger>(new AstInteger(a->location, a->module, ~static_cast<AVMInteger_t>(left->value))));
    case UnOp_positive:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, static_cast<ReturnValueType>(left->value))));
    case UnOp_negative:
      return std::move(std::unique_ptr<ReturnTypeAst>(new ReturnTypeAst(a->location, a->module, -static_cast<ReturnValueType>(left->value))));
    default:
      return nullptr;
    }
  }
};
} // namespace avm

#endif