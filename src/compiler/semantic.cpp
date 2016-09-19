#include <detail/semantic.h>

#include <memory>
#include <utility>
#include <iostream>
#include <fstream>

#include <alexer.h>
#include <aparser.h>
#include <config.h>

namespace avm {
SemanticAnalyzer::SemanticAnalyzer(CompilerState *state_ptr)
  : state_ptr(state_ptr) {
}

void SemanticAnalyzer::Analyze(AstModule *ast) {
  Accept(ast);

  LevelInfo &level = state_ptr->current_level();
  for (auto &&v : level.locals) {
    if (v.second.node != nullptr) {
      auto usecount = state_ptr->use_counts[v.second.node];
      if (usecount == 0) {
        WarningMessage(msg_unused_identifier, v.second.node->location, v.second.original_name);
      }
    }
  }

  state_ptr->block_id_counter = 0;
  state_ptr->level = compiler_global_level;
  state_ptr->levels.clear();
}

void SemanticAnalyzer::AddModule(const ModuleDefine &def) {
  // first, check that it wasn't already defined
  auto unit = std::unique_ptr<AstModule>(new AstModule(SourceLocation(-1, -1, ""), def.name));
  if (state_ptr->FindModule(def.name)) {
    ErrorMsg(msg_module_already_defined, unit->location, unit->name);
  } else {
    for (auto &&meth : def.methods) {
      std::string var_name = state_ptr->MakeVariableName(meth.name, unit.get());

      for (auto &&var : state_ptr->current_level().locals) {
        if (var.first == meth.name) {
          ErrorMsg(msg_redeclared_identifier, SourceLocation(-1, -1, ""));
          return;
        }
      }

      Symbol info;
      info.original_name = meth.name;
      info.nargs = meth.nargs;
      info.is_native = true;
      state_ptr->current_level().locals.push_back({ var_name, info });
    }

    state_ptr->other_modules[def.name] = std::move(unit);
  }
}

void SemanticAnalyzer::Accept(AstModule *node) {
  for (auto &&child : node->children) {
    Accept(child.get());
  }
}

void SemanticAnalyzer::Accept(AstNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case ast_type_imports:
    Accept(static_cast<AstImports*>(node));
    break;
  case ast_type_import:
    Accept(static_cast<AstImport*>(node));
    break;
  case ast_type_statement:
    Accept(static_cast<AstStatement*>(node));
    break;
  case ast_type_block:
    Accept(static_cast<AstBlock*>(node));
    break;
  case ast_type_expression:
    Accept(static_cast<AstExpression*>(node));
    break;
  case ast_type_binop:
    Accept(static_cast<AstBinaryOp*>(node));
    break;
  case ast_type_unop:
    Accept(static_cast<AstUnaryOp*>(node));
    break;
  case ast_type_array_access:
    Accept(static_cast<AstArrayAccess*>(node));
    break;
  case ast_type_member_access:
    Accept(static_cast<AstMemberAccess*>(node));
    break;
  case ast_type_module_access:
    Accept(static_cast<AstModuleAccess*>(node));
    break;
  case ast_type_var_declaration:
    Accept(static_cast<AstVariableDeclaration*>(node));
    break;
  case ast_type_alias:
    Accept(static_cast<AstAlias*>(node));
    break;
  case ast_type_use_module:
    Accept(static_cast<AstUseModule*>(node));
    break;
  case ast_type_variable:
    Accept(static_cast<AstVariable*>(node));
    break;
  case ast_type_integer:
    Accept(static_cast<AstInteger*>(node));
    break;
  case ast_type_float:
    Accept(static_cast<AstFloat*>(node));
    break;
  case ast_type_string:
    Accept(static_cast<AstString*>(node));
    break;
  case ast_type_true:
    Accept(static_cast<AstTrue*>(node));
    break;
  case ast_type_false:
    Accept(static_cast<AstFalse*>(node));
    break;
  case ast_type_null:
    Accept(static_cast<AstNull*>(node));
    break;
  case ast_type_self:
    Accept(static_cast<AstSelf*>(node));
    break;
  case ast_type_new:
    Accept(static_cast<AstNew*>(node));
    break;
  case ast_type_function_definition:
    Accept(static_cast<AstFunctionDefinition*>(node));
    break;
  case ast_type_function_expression:
    Accept(static_cast<AstFunctionExpression*>(node));
    break;
  case ast_type_function_call:
    Accept(static_cast<AstFunctionCall*>(node));
    break;
  case ast_type_class_declaration:
    Accept(static_cast<AstClass*>(node));
    break;
  case ast_type_object_expression:
    Accept(static_cast<AstObjectExpression*>(node));
    break;
  case ast_type_enum:
    Accept(static_cast<AstEnum*>(node));
    break;
  case ast_type_print:
    Accept(static_cast<AstPrintStmt*>(node));
    break;
  case ast_type_return:
    Accept(static_cast<AstReturnStmt*>(node));
    break;
  case ast_type_if_statement:
    Accept(static_cast<AstIfStmt*>(node));
    break;
  case ast_type_for_loop:
    Accept(static_cast<AstForLoop*>(node));
    break;
  case ast_type_while_loop:
    Accept(static_cast<AstWhileLoop*>(node));
    break;
  case ast_type_try_catch:
    Accept(static_cast<AstTryCatch*>(node));
    break;
  default:
    ErrorMsg(msg_internal_error, node->location);
    break;
  }
}

void SemanticAnalyzer::Accept(AstImports *node) {
  for (auto &child : node->children) {
    Accept(child.get());
  }
}

void SemanticAnalyzer::Accept(AstImport *node) {
  if (state_ptr->level != compiler_global_level) {
    ErrorMsg(msg_import_outside_global, node->location);
  }

  // load relative file
  std::string path = node->relative_path + node->import_str;

  // Check if the module has already been imported
  if (state_ptr->other_modules.find(path) == state_ptr->other_modules.end()) {
    std::ifstream file(path);

    if (!file.is_open()) {
      ErrorMsg(msg_import_not_found, node->location, node->import_str, path);
    } else {
      std::string str((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

      file.close();

      Lexer l(str, path);
      auto tokens = l.ScanTokens();

      Parser p(tokens, l.state);
      auto unit = p.Parse();

      bool already_imported = false;
      for (auto &&it : state_ptr->other_modules) {
        if (it.second->name == unit->name) {
          already_imported = true;
          break;
        }
      }

      if (!already_imported) {
        state_ptr->other_modules[path] = std::move(unit);

        for (auto &&error : p.state.errors) {
          state_ptr->errors.push_back(error);
        }

        auto &module = state_ptr->other_modules[path];
        if (module != nullptr) {
          for (auto &&child : module->children) {
            Accept(child.get());
          }
        }
      } else {
        ErrorMsg(msg_module_already_defined, node->location, unit->name);
      }
    }
  }
}

void SemanticAnalyzer::Accept(AstStatement *node) {
}

void SemanticAnalyzer::Accept(AstBlock *node) {
  bool in_dead_code = false;
  bool warning_shown = false;

  size_t counter = 0;
  while (counter < node->children.size()) {
    auto &child = node->children[counter];
    Accept(child.get());

    if (child) {
      if (child->type == ast_type_return) {
        in_dead_code = true;
      } else if (in_dead_code && !warning_shown && child->type != ast_type_statement) {
        WarningMessage(msg_unreachable_code, child->location);
        warning_shown = true;
      }
    }

    ++counter;
  }
}

void SemanticAnalyzer::Accept(AstExpression *node) {
  Accept(node->child.get());
}

void SemanticAnalyzer::Accept(AstBinaryOp *node) {
  auto &left = node->left;
  auto &right = node->right;

  Accept(left.get());
  Accept(right.get());

  switch (node->op) {
    // FALLTHROUGH
  case BinOp_assign:
    // assignment, change the set type
    if (left->type == ast_type_variable) {
      auto *casted = static_cast<AstVariable*>(left.get());
      if (!casted->is_const) {
        auto *right_side = right.get();
        if (right->type == ast_type_expression) {
          // get inner child
          auto *expr_ast = static_cast<AstExpression*>(right.get());
          right_side = expr_ast->child.get();
        }

        std::unique_ptr<AstNode> right_side_opt = nullptr;
        if (config::optimize_constant_folding) {
          right_side_opt = right_side->Optimize();
          if (right_side_opt != nullptr) {
            right_side = right_side_opt.get();
          }
        }

        casted->symbol_ptr->current_value = right.get();
        casted->current_value = casted->symbol_ptr->current_value;
        switch (right_side->type) {
        case ast_type_integer:
        case ast_type_float:
        case ast_type_string:
          casted->symbol_ptr->is_literal = true;
          break;
        default:
          casted->symbol_ptr->is_literal = false;
          break;
        }
      }
    }
  case BinOp_add_assign:
  case BinOp_subtract_assign:
  case BinOp_multiply_assign:
  case BinOp_divide_assign:
    if (left->type == ast_type_variable) {
      auto *casted = static_cast<AstVariable*>(left.get());
      if (casted->is_const) {
        ErrorMsg(msg_const_identifier, casted->location, casted->name);
      }

      // This usage is prohibited on inlined objects
      if (left->HasAttribute("inline")) {
        ErrorMsg(msg_prohibited_action_attribute, left->location, "inline");
      }
      if (right->HasAttribute("inline")) {
        ErrorMsg(msg_prohibited_action_attribute, right->location, "inline");
      }
    } else if (left->type == ast_type_member_access) {
      /// \todo: check const
    } else if (left->type == ast_type_array_access) {
      /// \todo: check const
    } else {
      ErrorMsg(msg_expected_identifier, left->location);
    }
    break;
  }
}

void SemanticAnalyzer::Accept(AstUnaryOp *node) {
  Accept(node->child.get());
}

void SemanticAnalyzer::Accept(AstArrayAccess *node) {
  Accept(node->object.get());
  Accept(node->index.get());
}

void SemanticAnalyzer::Accept(AstMemberAccess *node) {
  // checks for module with name first
  AstModule *found_module = nullptr;
  if (state_ptr->FindModule(node->left_str, node->module, found_module)) {
    // it is a module being referenced
    // set the right node's module to be the one we found
    node->right->module = found_module;
    Accept(node->right.get());
  } else {
    Accept(node->left.get());
    if (node->right->type == ast_type_member_access) {
      Accept(node->right.get());
    } else if (node->right->type != ast_type_variable && node->right->type != ast_type_function_call) {
      ErrorMsg(msg_internal_error, node->location);
    }
  }
}

void SemanticAnalyzer::Accept(AstModuleAccess *node) {
  for (auto &&mod : state_ptr->other_modules) {
    if (mod.second->name == node->module_name) {
      node->right->module = mod.second.get();
      Accept(node->right.get());
      return;
    }
  }
  ErrorMsg(ErrorType::msg_module_not_imported, node->location, node->module_name);
}

void SemanticAnalyzer::Accept(AstVariableDeclaration *node) {
  //node->is_const = node->HasAttribute("const");

  std::string var_name = state_ptr->MakeVariableName(node->name, node->module);
  if (FindVariable(var_name, true)) {
    ErrorMsg(msg_redeclared_identifier, node->location, node->name);
  } else if (state_ptr->FindModule(node->name, node->module)) {
    ErrorMsg(msg_identifier_is_module, node->location, node->name);
  } else {
    Symbol info;
    info.node = node;
    info.original_name = node->name;
    info.is_const = node->is_const;
    info.current_value = node->assignment.get();

    auto *right_side = info.current_value;
    if (info.current_value->type == ast_type_expression) {
      // get inner child
      auto *expr_ast = static_cast<AstExpression*>(info.current_value);
      right_side = expr_ast->child.get();
    }

    std::unique_ptr<AstNode> right_side_opt = nullptr;
    if (config::optimize_constant_folding) {
      right_side_opt = right_side->Optimize();
      if (right_side_opt != nullptr) {
        right_side = right_side_opt.get();
      }
    }

    switch (right_side->type) {
    case ast_type_integer:
    case ast_type_float:
    case ast_type_string:
      info.is_literal = true;
      break;
    default:
      info.is_literal = false;
      break;
    }

    state_ptr->current_level().locals.push_back({ var_name, info });

    Accept(node->assignment.get());
  }
}

void SemanticAnalyzer::Accept(AstAlias *node) {
  std::string var_name = state_ptr->MakeVariableName(node->name, node->module);
  if (FindVariable(var_name, true)) {
    ErrorMsg(msg_redeclared_identifier, node->location, node->name);
  } else if (state_ptr->FindModule(node->name, node->module)) {
    ErrorMsg(msg_identifier_is_module, node->location, node->name);
  } else {
    Accept(node->alias_to.get());

    Symbol info;
    info.node = node->alias_to.get();
    info.original_name = node->name;
    info.is_alias = true;
    state_ptr->current_level().locals.push_back({ var_name, info });
  }
}

void SemanticAnalyzer::Accept(AstUseModule *node) {
  /*for (auto &&it : state_ptr->levels[0].locals) {
    AstModule *module = nullptr;
    if (it.second.node && it.second.node->module &&
        it.second.node->module->type == ast_type_module) {
      module = static_cast<AstModule*>(it.second.node->module);
      if (module->name == node->name) {
        // add an alias for this object
        std::string var_name = state_ptr->MakeVariableName(it.second.original_name, node->module);
        if (FindVariable(var_name, true)) {
          ErrorMsg(msg_redeclared_identifier, node->location, it.second.original_name);
        } else if (state_ptr->FindModule(node->name, node->module)) {
          ErrorMsg(msg_identifier_is_module, node->location, it.second.original_name);
        } else {
          Symbol info;
          info.node = it.second.node;
          info.original_name = it.second.original_name;
          info.is_alias = true;
          state_ptr->current_level().locals.push_back({ var_name, info });
        }
      }
    }
  }*/
  ErrorMsg(msg_unsupported_feature, node->location);
}

void SemanticAnalyzer::Accept(AstVariable *node) {
  Symbol *ptr = nullptr;
  std::string var_name = state_ptr->MakeVariableName(node->name, node->module);
  if (FindVariable(var_name, false, ptr)) {
    // Copy symbol information
    node->is_alias = ptr->is_alias;
    if (ptr->is_alias) {
      node->alias_to = ptr->node;
    }
    node->is_const = ptr->is_const;
    node->is_literal = ptr->is_literal;
    node->current_value = ptr->current_value;
    node->symbol_ptr = ptr;

    if (ptr->node != nullptr) {
      if (ptr->node->type == ast_type_function_definition) {
        if (ptr->node->HasAttribute("inline")) {
          ErrorMsg(msg_prohibited_action_attribute, node->location, "inline");
        }
      }

      // do not increment use count for const literals, they will be inlined
      if (!(config::optimize_constant_folding &&
        node->is_const && node->is_literal && node->current_value != nullptr)) {
        IncrementUseCount(ptr->node);
      }
    }
  } else {
    ErrorMsg(msg_undeclared_identifier, node->location, node->name);
  }
}

void SemanticAnalyzer::Accept(AstInteger *node) {
}

void SemanticAnalyzer::Accept(AstFloat *node) {
}

void SemanticAnalyzer::Accept(AstString *node) {
}

void SemanticAnalyzer::Accept(AstTrue *node) {
}

void SemanticAnalyzer::Accept(AstFalse *node) {
}

void SemanticAnalyzer::Accept(AstNull *node) {
}

void SemanticAnalyzer::Accept(AstSelf *node) {
}

void SemanticAnalyzer::Accept(AstNew *node) {
}

void SemanticAnalyzer::Accept(AstFunctionDefinition *node) {
  std::string var_name = state_ptr->MakeVariableName(node->name, node->module);
  if (FindVariable(var_name, true)) {
    ErrorMsg(msg_redeclared_identifier, node->location, node->name);
  } else if (state_ptr->FindModule(node->name, node->module)) {
    ErrorMsg(msg_identifier_is_module, node->location, node->name);
  } else {
    if (!node->HasAttribute("inline")) {
      Symbol info;
      info.node = node;
      info.original_name = node->name;
      state_ptr->current_level().locals.push_back({ var_name, info });
    }

    AstBlock *body = dynamic_cast<AstBlock*>(node->block.get());
    if (body) {
      if (body->children.empty()) {
        SourceLocation loc = body->location;

        // show warnings
        InfoMsg(msg_empty_function_body, loc, node->name);
        InfoMsg(msg_missing_final_return, node->location, node->name);

        // add return statement
        auto ret_value = std::unique_ptr<AstNull>(new AstNull(loc, node->module));
        auto ret_ast = std::unique_ptr<AstReturnStmt>(new AstReturnStmt(loc, node->module, std::move(ret_value)));

        body->AddChild(std::move(ret_ast));
      } else {
        bool has_return = false;

        if (!body->children.empty()) {
          size_t idx = body->children.size() - 1;
          if (body->children[idx] && body->children[idx]->type == ast_type_return) {
            has_return = true;
          } else {
            while (idx > 0 && body->children[idx]->type == ast_type_statement) {
              if (body->children[idx - 1]->type == ast_type_return) {
                has_return = true;
                break;
              } else if (body->children[idx - 1]->type != ast_type_statement) {
                has_return = false;
                break;
              }

              --idx;
            }
          }
        }

        if (!has_return) {
          SourceLocation loc = body->children.back() ? body->children.back()->location : body->location;
          // show warning
          InfoMsg(msg_missing_final_return, node->location, node->name);

          // add return statement
          auto ret_value = std::unique_ptr<AstNull>(new AstNull(loc, node->module));
          auto ret_ast = std::unique_ptr<AstReturnStmt>(new AstReturnStmt(loc, node->module, std::move(ret_value)));

          body->AddChild(std::move(ret_ast));
        }
      }

      IncreaseBlock(LevelType::Level_function);

      // declare a variable for all parameters
      for (auto &&param : node->arguments) {
        Symbol info;
        info.node = nullptr;
        info.original_name = param;
        std::string var_name = state_ptr->MakeVariableName(param, node->module);
        state_ptr->current_level().locals.push_back({ var_name, info });
      }

      Accept(body);
      DecreaseBlock();

      if (node->HasAttribute("inline")) {
        // Inline functions cannot be recursive, so we will declare
        // the symbol here to avoid recursive usage.
        Symbol info;
        info.node = node;
        info.original_name = node->name;
        info.is_const = true;
        state_ptr->current_level().locals.push_back({ var_name, info });
      }
    }
  }
}

void SemanticAnalyzer::Accept(AstFunctionExpression *node) {
  std::string name = "unnamed";
  AstBlock *body = dynamic_cast<AstBlock*>(node->block.get());
  if (body) {
    if (body->children.empty()) {
      SourceLocation loc = body->location;

      // show warnings
      InfoMsg(msg_empty_function_body, loc, name);
      InfoMsg(msg_missing_final_return, node->location, name);

      // add return statement
      auto ret_value = std::unique_ptr<AstNull>(new AstNull(loc, node->module));
      auto ret_ast = std::unique_ptr<AstReturnStmt>(new AstReturnStmt(loc, node->module, std::move(ret_value)));

      body->AddChild(std::move(ret_ast));
    } else {
      bool has_return = false;
      size_t idx = body->children.size() - 1;

      if (body->children[idx]->type == ast_type_return) {
        has_return = true;
      } else {
        while (idx > 0 && body->children[idx]->type == ast_type_statement) {
          if (body->children[idx - 1]->type == ast_type_return) {
            has_return = true;
            break;
          } else if (body->children[idx - 1]->type != ast_type_statement) {
            has_return = false;
            break;
          }

          --idx;
        }
      }

      if (!has_return) {
        SourceLocation loc = body->children.back() ? body->children.back()->location : body->location;

        // show warning
        InfoMsg(msg_missing_final_return, node->location, name);

        // add return statement
        auto ret_value = std::unique_ptr<AstNull>(new AstNull(loc, node->module));
        auto ret_ast = std::unique_ptr<AstReturnStmt>(new AstReturnStmt(loc, node->module, std::move(ret_value)));

        body->AddChild(std::move(ret_ast));
      }
    }

    IncreaseBlock(LevelType::Level_function);

    // declare a variable for all parameters
    for (auto &&param : node->arguments) {
      Symbol info;
      info.node = node;
      info.original_name = param;
      std::string var_name = state_ptr->MakeVariableName(param, node->module);
      state_ptr->current_level().locals.push_back({ var_name, info });
    }

    Accept(body);
    DecreaseBlock();
  }
}

void SemanticAnalyzer::Accept(AstFunctionCall *node) {
  Symbol *ptr = nullptr;
  std::string var_name = state_ptr->MakeVariableName(node->name, node->module);
  if (FindVariable(var_name, false, ptr)) {
    if (ptr->is_alias) {
      node->is_alias = true;
      node->alias_to = ptr->node;
    }

    node->definition = ptr->node;

    IncrementUseCount(ptr->node);

    for (int i = node->arguments.size() - 1; i >= 0; i--) {
      // Push each argument onto the stack
      Accept(node->arguments[i].get());
    }
  } else {
    ErrorMsg(msg_undeclared_identifier, node->location, node->name);
  }
}

void SemanticAnalyzer::Accept(AstClass *node) {
}

void SemanticAnalyzer::Accept(AstObjectExpression *node) {
  for (auto &&mem : node->members) {
    Accept(mem.second.get());
  }
}

void SemanticAnalyzer::Accept(AstEnum *node) {
  // currently, the enum identifier is not created, only the members of it
  /*std::string var_name = state_ptr->MakeVariableName(node->name, node->module);
  if (FindVariable(var_name, true)) {
    ErrorMsg(msg_redeclared_identifier, node->location, node->name);
  } else if (state_ptr->FindModule(node->name, node->module)) {
    ErrorMsg(msg_identifier_is_module, node->location, node->name);
  } else {
    Symbol info;
    info.node = node;
    info.original_name = node->name;
    state_ptr->current_level().locals.push_back({ var_name, info });
  }*/
  for (auto &&it : node->members) {
    std::string var_name = state_ptr->MakeVariableName(it.first, it.second->module);
    if (FindVariable(var_name, true)) {
      ErrorMsg(msg_redeclared_identifier, it.second->location, it.first);
    } else if (state_ptr->FindModule(it.first, it.second->module)) {
      ErrorMsg(msg_identifier_is_module, it.second->location, it.first);
    } else {
      Symbol info;
      info.node = it.second.get();
      info.original_name = it.first;
      info.is_alias = true;
      info.is_const = true;
      state_ptr->current_level().locals.push_back({ var_name, info });
    }
  }
}

void SemanticAnalyzer::Accept(AstIfStmt *node) {
  Accept(node->conditional.get());

  IncreaseBlock(LevelType::Level_condition);
  Accept(node->block.get());
  DecreaseBlock();

  if (node->else_statement) {
    IncreaseBlock(LevelType::Level_condition);
    Accept(node->else_statement.get());
    DecreaseBlock();
  }
}

void SemanticAnalyzer::Accept(AstPrintStmt *node) {
  for (auto &&arg : node->arguments) {
    Accept(arg.get());
  }
}

void SemanticAnalyzer::Accept(AstReturnStmt *node) {
  // The resulting value will get pushed onto the stack
  Accept(node->value.get());

  int start = state_ptr->level;
  LevelInfo *level = &state_ptr->levels[start];
  while (start >= compiler_global_level && level->type != LevelType::Level_function) {
    level = &state_ptr->levels[--start];
  }
}

void SemanticAnalyzer::Accept(AstForLoop *node) {
  /*std::string var_name = state_ptr->MakeVariableName(node->identifier, node->module);

  // create the index variable
  Symbol info;
  info.node = nullptr;
  info.original_name = node->identifier;
  state_ptr->current_level().locals.push_back({ var_name, info });*/

  if (node->block != nullptr) {
    auto *block = dynamic_cast<AstBlock*>(node->block.get());
    if (block != nullptr && block->children.empty()) {
      InfoMsg(msg_empty_statement_body, block->location);
    }
  }

  Accept(node->initializer.get());
  Accept(node->conditional.get());

  IncreaseBlock(LevelType::Level_loop);
  Accept(node->block.get());
  DecreaseBlock();

  Accept(node->afterthought.get());
}

void SemanticAnalyzer::Accept(AstWhileLoop *node) {
  Accept(node->conditional.get());

  if (node->block != nullptr) {
    auto *block = dynamic_cast<AstBlock*>(node->block.get());
    if (block != nullptr && block->children.empty()) {
      InfoMsg(msg_empty_statement_body, block->location);
    }
  }

  IncreaseBlock(LevelType::Level_loop);
  Accept(node->block.get());
  DecreaseBlock();
}

void SemanticAnalyzer::Accept(AstTryCatch *node) {
  if (node->try_block != nullptr) {
    auto *block = dynamic_cast<AstBlock*>(node->try_block.get());
    if (block != nullptr && block->children.empty()) {
      InfoMsg(msg_empty_statement_body, block->location);
    }
  }

  IncreaseBlock(LevelType::Level_default);
  Accept(node->try_block.get());
  DecreaseBlock();

  if (node->catch_block != nullptr) {
    auto *block = dynamic_cast<AstBlock*>(node->catch_block.get());
    if (block != nullptr && block->children.empty()) {
      InfoMsg(msg_empty_statement_body, block->location);
    }
  }

  IncreaseBlock(LevelType::Level_default);
  Accept(node->exception_object.get());
  Accept(node->catch_block.get());
  DecreaseBlock();
}

void SemanticAnalyzer::Accept(AstRange *node) {
}

void SemanticAnalyzer::IncrementUseCount(AstNode *ptr) {
  if (state_ptr->use_counts.find(ptr) != state_ptr->use_counts.end()) {
    ++state_ptr->use_counts[ptr];
  } else {
    state_ptr->use_counts.insert({ ptr, 1 });
  }
}

bool SemanticAnalyzer::FindVariable(const std::string &name, bool only_this_scope) {
  Symbol *tmp = nullptr;
  return FindVariable(name, only_this_scope, tmp);
}

bool SemanticAnalyzer::FindVariable(const std::string &name, bool only_this_scope, Symbol *&out) {
  // start at current level
  int start = state_ptr->level;
  while (start >= compiler_global_level) {
    auto &level = state_ptr->levels[start];
    auto it = std::find_if(level.locals.begin(), level.locals.end(),
      [&name](const std::pair<std::string, Symbol> &elt) {
      return elt.first == name;
    });

    if (it != level.locals.end()) {
      out = &it->second;
      return true;
    }

    if (only_this_scope) {
      break;
    }
    start--;
  }
  return false;
}

/*bool SemanticAnalyzer::FindVariable(const std::string &name, size_t nargs, bool only_this_scope, Symbol *&out) {
  // start at current level
  int start = state_ptr->level;
  while (start >= compiler_global_level) {
    auto &level = state_ptr->levels[start];
    auto it = std::find_if(level.locals.begin(), level.locals.end(),
      [&name, &nargs](const std::pair<std::string, Symbol> &elt) {
      if (elt.first == name) {
        if (elt.second.is_native && elt.second.nargs == nargs) {
          return true;
        } else if (!elt.second.is_native) {
          return true;
        }
      }
      return false;
    });

    if (it != level.locals.end()) {
      out = &it->second;
      return true;
    }

    if (only_this_scope) {
      break;
    }
    --start;
  }
  return false;
}*/

void SemanticAnalyzer::IncreaseBlock(LevelType type) {
  LevelInfo level;
  level.type = type;
  state_ptr->levels[++state_ptr->level] = level;
}

void SemanticAnalyzer::DecreaseBlock() {
  LevelInfo &level = state_ptr->current_level();
  for (auto &&v : level.locals) {
    if (v.second.node != nullptr) {
      auto usecount = state_ptr->use_counts[v.second.node];
      if (usecount == 0) {
        WarningMessage(msg_unused_identifier, v.second.node->location, v.second.original_name);
      }
    }
  }
  state_ptr->levels[state_ptr->level--] = LevelInfo();
}
} // namespace avm