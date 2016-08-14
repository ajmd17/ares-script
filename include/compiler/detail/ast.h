#ifndef AST_H
#define AST_H

#include <string>
#include <utility>
#include <memory>
#include <vector>
#include <map>

#include <common/types.h>
#include <detail/location.h>
#include <detail/syntax/keywords.h>
#include <detail/syntax/operators.h>

namespace avm {
enum AstType {
  ast_type_module,
  ast_type_imports,
  ast_type_import,
  ast_type_statement,
  ast_type_block,
  ast_type_expression,
  ast_type_binop,
  ast_type_unop,
  ast_type_array_access,
  ast_type_member_access,
  ast_type_module_access,
  ast_type_var_declaration,
  ast_type_variable,
  ast_type_alias,
  ast_type_use_module,
  ast_type_integer,
  ast_type_float,
  ast_type_string,
  ast_type_true,
  ast_type_false,
  ast_type_null,
  ast_type_self,
  ast_type_new,
  ast_type_function_definition,
  ast_type_function_expression,
  ast_type_function_call,
  ast_type_class_declaration,
  ast_type_enum,
  ast_type_if_statement,
  ast_type_print,
  ast_type_return,
  ast_type_for_loop,
  ast_type_while_loop,
  ast_type_try_catch,
};

struct AstClass;

struct AstNode {
public:
  AstNode *module = nullptr;
  std::pair<AVMString_t, AstClass*> self = { "", nullptr };

  std::vector<std::string> attributes;
  const SourceLocation location;
  const AstType type;

  AstNode(SourceLocation location, AstNode *module, AstType type)
    : location(location), module(module), type(type) {
  }

  virtual ~AstNode() {}

  bool HasAttribute(const std::string &attr) const {
    return std::find(attributes.begin(), attributes.end(), attr) != attributes.end();
  }
};

struct AstModule : public AstNode {
  AVMString_t name;
  std::vector<std::unique_ptr<AstNode>> children;

  AstModule(SourceLocation location, const AVMString_t &name)
    : name(name), AstNode(location, this, ast_type_module) {
  }

  void AddChild(std::unique_ptr<AstNode> node) {
    children.push_back(std::move(node));
  }
};

struct AstStatement : public AstNode {
  std::vector<std::unique_ptr<AstNode>> children;

  AstStatement(SourceLocation location, AstNode *module)
    : AstNode(location, module, ast_type_statement) {
  }

  void AddChild(std::unique_ptr<AstNode> node) {
    children.push_back(std::move(node));
  }
};

struct AstImports : public AstNode {
  std::vector<std::unique_ptr<AstNode>> children;

  AstImports(SourceLocation location, AstNode *module,
    std::vector<std::unique_ptr<AstNode>> children)
    : children(std::move(children)), AstNode(location, module, ast_type_imports) {
  }
};

struct AstImport : public AstNode {
  AVMString_t import_str;
  AVMString_t relative_path;
  bool is_module_import;

  AstImport(SourceLocation location, AstNode *module,
    const AVMString_t &import_str, const AVMString_t &relative_path,
    bool is_module_import)
    : import_str(import_str),
    relative_path(relative_path),
    is_module_import(is_module_import),
    AstNode(location, module, ast_type_import) {
  }
};

struct AstBlock : public AstNode {
  std::vector<std::unique_ptr<AstNode>> children;

  AstBlock(SourceLocation location, AstNode *module)
    : AstNode(location, module, ast_type_block) {
  }

  void AddChild(std::unique_ptr<AstNode> node) {
    children.push_back(std::move(node));
  }
};

struct AstExpression : public AstNode {
  std::unique_ptr<AstNode> child;
  bool should_clear_stack;

  AstExpression(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> child, bool should_clear_stack)
    : child(move(child)), should_clear_stack(should_clear_stack),
    AstNode(location, module, AstType::ast_type_expression) {
  }
};

struct AstBinaryOp : public AstNode {
  std::unique_ptr<AstNode> left;
  std::unique_ptr<AstNode> right;
  st_binary_operator op;

  AstBinaryOp(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> left, std::unique_ptr<AstNode> right,
    st_binary_operator op)
    : left(move(left)), right(move(right)), op(op),
    AstNode(location, module, ast_type_binop) {
  }
};

struct AstUnaryOp : public AstNode {
  std::unique_ptr<AstNode> child;
  st_unary_operator op;

  AstUnaryOp(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> child, st_unary_operator op)
    : child(move(child)), op(op),
    AstNode(location, module, ast_type_unop) {
  }
};

struct AstArrayAccess : public AstNode {
  std::unique_ptr<AstNode> object;
  std::unique_ptr<AstNode> index;

  AstArrayAccess(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> object, std::unique_ptr<AstNode> index)
    : object(std::move(object)), index(std::move(index)),
    AstNode(location, module, ast_type_array_access) {
  }
};

struct AstMemberAccess : public AstNode {
  AVMString_t left_str;
  std::unique_ptr<AstNode> left;
  std::unique_ptr<AstNode> right;

  AstMemberAccess(SourceLocation location, AstNode *module,
    const AVMString_t &left_str,
    std::unique_ptr<AstNode> left, std::unique_ptr<AstNode> right)
    : left_str(left_str), left(std::move(left)), right(std::move(right)),
    AstNode(location, module, ast_type_member_access) {
  }
};

struct AstModuleAccess : public AstNode {
  AVMString_t module_name;
  std::unique_ptr<AstNode> right;

  AstModuleAccess(SourceLocation location, AstNode *module,
    const AVMString_t &module_name, std::unique_ptr<AstNode> right)
    : module_name(module_name), right(std::move(right)),
    AstNode(location, module, ast_type_module_access) {
  }
};

struct AstVariableDeclaration : public AstNode {
  AVMString_t name;
  std::unique_ptr<AstNode> assignment;
  bool is_const;

  AstVariableDeclaration(SourceLocation location, AstNode *module,
    const AVMString_t &name, std::unique_ptr<AstNode> assignment, bool is_const)
    : name(name), assignment(std::move(assignment)), is_const(is_const),
    AstNode(location, module, ast_type_var_declaration) {
  }
};

struct AstAlias : public AstNode {
  AVMString_t name;
  std::unique_ptr<AstNode> alias_to;

  AstAlias(SourceLocation location, AstNode *module,
    const AVMString_t &name, std::unique_ptr<AstNode> alias_to)
    : name(name), alias_to(std::move(alias_to)),
    AstNode(location, module, ast_type_alias) {
  }
};

struct AstUseModule : public AstNode {
  AVMString_t name;

  AstUseModule(SourceLocation location, AstNode *module, const AVMString_t &name)
    : name(name), AstNode(location, module, ast_type_use_module) {
  }
};

struct AstVariable : public AstNode {
  AVMString_t name;

  /* ===== To be set by semantic analyzer ===== */
  bool is_alias = false;
  AstNode *alias_to = nullptr;
  bool is_const = false; 
  /* ========================================== */

  AstVariable(SourceLocation location, AstNode *module, const AVMString_t &name)
    : name(name), AstNode(location, module, ast_type_variable) {
  }
};

struct AstInteger : public AstNode {
  AVMInteger_t value;

  AstInteger(SourceLocation location, AstNode *module, AVMInteger_t value)
    : value(value), AstNode(location, module, ast_type_integer) {
  }
};

struct AstFloat : public AstNode {
  AVMFloat_t value;

  AstFloat(SourceLocation location, AstNode *module, AVMFloat_t value)
    : value(value), AstNode(location, module, ast_type_float) {
  }
};

struct AstString : public AstNode {
  AVMString_t value;

  AstString(SourceLocation location, AstNode *module, const AVMString_t &value)
    : value(value), AstNode(location, module, ast_type_string) {
  }
};

struct AstTrue : public AstNode {
  AstTrue(SourceLocation location, AstNode *module)
    : AstNode(location, module, ast_type_true) {
  }
};

struct AstFalse : public AstNode {
  AstFalse(SourceLocation location, AstNode *module)
    : AstNode(location, module, ast_type_false) {
  }
};

struct AstNull : public AstNode {
  AstNull(SourceLocation location, AstNode *module)
    : AstNode(location, module, ast_type_null) {
  }
};

struct AstSelf : public AstNode {
  AstSelf(SourceLocation location, AstNode *module)
    : AstNode(location, module, ast_type_self) {
  }
};

struct AstNew : public AstNode {
  AVMString_t identifier;
  std::unique_ptr<AstNode> constructor;

  AstNew(SourceLocation location, AstNode *module,
    const AVMString_t &identifier, // the variable it will go into
    std::unique_ptr<AstNode> constructor)
    : identifier(identifier), constructor(std::move(constructor)),
    AstNode(location, module, ast_type_new) {
  }
};

struct AstFunctionDefinition : public AstNode {
  AVMString_t name;
  std::vector<AVMString_t> arguments;
  std::unique_ptr<AstNode> block;
  bool is_native;

  AstFunctionDefinition(SourceLocation location, AstNode *module,
    const AVMString_t &name, std::vector<AVMString_t> arguments,
    std::unique_ptr<AstNode> block, bool is_native = false)
    : name(name), arguments(arguments), block(std::move(block)), is_native(is_native),
    AstNode(location, module, ast_type_function_definition) {
  }
};

struct AstFunctionExpression : public AstNode {
  std::vector<AVMString_t> arguments;
  std::unique_ptr<AstNode> block;

  AstFunctionExpression(SourceLocation location, AstNode *module,
    std::vector<AVMString_t> arguments, std::unique_ptr<AstNode> block)
    : arguments(arguments), block(std::move(block)),
    AstNode(location, module, ast_type_function_expression) {
  }
};

struct AstFunctionCall : public AstNode {
  AVMString_t name;
  std::vector<std::unique_ptr<AstNode>> arguments;

  /* ===== Set by the analyzer ===== */
  bool is_alias = false;
  AstNode *alias_to = nullptr;
  AstNode *definition = nullptr;
  /* =============================== */

  AstFunctionCall(SourceLocation location, AstNode *module,
    const AVMString_t &name, std::vector<std::unique_ptr<AstNode>> arguments)
    : name(name), arguments(std::move(arguments)),
    AstNode(location, module, ast_type_function_call) {
  }

  void AddArgument(std::unique_ptr<AstNode> arg) {
    arguments.push_back(std::move(arg));
  }
};

struct AstClass : public AstNode {
  AVMString_t name;
  std::vector<std::unique_ptr<AstNode>> members;

  AstClass(SourceLocation location, AstNode *module,
    const AVMString_t &name, std::vector<std::unique_ptr<AstNode>> members)
    : name(name), members(std::move(members)),
    AstNode(location, module, ast_type_class_declaration) {
  }
};

struct AstEnum : public AstNode {
  AVMString_t name;
  std::vector<std::pair<AVMString_t, std::unique_ptr<AstInteger>>> members;

  AstEnum(SourceLocation location, AstNode *module, const AVMString_t &name,
    std::vector<std::pair<AVMString_t, std::unique_ptr<AstInteger>>> members)
    : name(name), members(std::move(members)),
    AstNode(location, module, ast_type_enum) {
  }
};

struct AstIfStmt : public AstNode {
  std::unique_ptr<AstNode> conditional, block, else_statement;

  AstIfStmt(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> conditional, std::unique_ptr<AstNode> block,
    std::unique_ptr<AstNode> else_statement)
    : conditional(std::move(conditional)), block(std::move(block)),
    else_statement(std::move(else_statement)),
    AstNode(location, module, ast_type_if_statement) {
  }
};

struct AstPrintStmt : public AstNode {
  std::vector<std::unique_ptr<AstNode>> arguments;

  AstPrintStmt(SourceLocation location, AstNode *module)
    : arguments(std::move(arguments)),
    AstNode(location, module, ast_type_print) {
  }

  void AddArgument(std::unique_ptr<AstNode> arg) {
    arguments.push_back(std::move(arg));
  }
};

struct AstReturnStmt : public AstNode {
  std::unique_ptr<AstNode> value;

  AstReturnStmt(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> value)
    : value(std::move(value)),
    AstNode(location, module, AstType::ast_type_return) {
  }
};

struct AstForLoop : public AstNode {
  std::unique_ptr<AstNode> initializer;
  std::unique_ptr<AstNode> conditional;
  std::unique_ptr<AstNode> afterthought;
  std::unique_ptr<AstNode> block;

  AstForLoop(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> initializer,
    std::unique_ptr<AstNode> conditional,
    std::unique_ptr<AstNode> afterthought,
    std::unique_ptr<AstNode> block)
    : initializer(std::move(initializer)),
    conditional(std::move(conditional)),
    afterthought(std::move(afterthought)),
    block(std::move(block)),
    AstNode(location, module, ast_type_for_loop) {
  }
};

struct AstWhileLoop : public AstNode {
  std::unique_ptr<AstNode> conditional;
  std::unique_ptr<AstNode> block;

  AstWhileLoop(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> conditional, std::unique_ptr<AstNode> block)
    : conditional(std::move(conditional)), block(std::move(block)),
    AstNode(location, module, ast_type_while_loop) {
  }
};

struct AstTryCatch : public AstNode {
  std::unique_ptr<AstNode> try_block;
  std::unique_ptr<AstNode> catch_block;
  std::unique_ptr<AstNode> exception_object;

  AstTryCatch(SourceLocation location, AstNode *module,
    std::unique_ptr<AstNode> try_block,
    std::unique_ptr<AstNode> catch_block,
    std::unique_ptr<AstNode> exception_object)
    : try_block(std::move(try_block)),
    catch_block(std::move(catch_block)),
    exception_object(std::move(exception_object)),
    AstNode(location, module, ast_type_try_catch) {
  }
};
} // namespace avm

#endif