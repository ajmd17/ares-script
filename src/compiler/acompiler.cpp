#include <acompiler.h>

#include <memory>
#include <utility>
#include <iostream>
#include <algorithm>
#include <fstream>

#include <detail/semantic.h>
#include <config.h>
#include <alexer.h>
#include <aparser.h>

namespace avm {
Compiler::Compiler(const ParserState &parser_state) {
  for (auto &&error : parser_state.errors) {
    state.errors.push_back(error);
  }
}

CompilerState &Compiler::GetState() {
  return state;
}

InstructionStream &Compiler::GetInstructions() {
  return bstream;
}

ModuleDefine &Compiler::Module(const std::string &name) {
  ModuleDefine mod;
  mod.name = name;
  native_modules.push_back(mod);
  return native_modules.back();
}

bool Compiler::Compile(AstModule *node) {
  SemanticAnalyzer semantic(&state);
  for (auto &&it : native_modules) {
    semantic.AddModule(it);
  }

  semantic.Analyze(node);

  bool has_fatal_errors = false;
  if (!state.errors.empty()) {
    // maps file to errors
    std::map< std::string, std::vector< BuildMessage > > errors;

    for (auto &&it : state.errors) {
      if (it.level == err_fatal) {
        has_fatal_errors = true;
      }

      if (errors.find(it.location.file) == errors.end()) {
        errors.insert({ it.location.file, {} });
      }

      errors[it.location.file].push_back(it);
    }

    for (auto it = errors.rbegin(); it != errors.rend(); ++it) {
      std::sort(it->second.begin(), it->second.end());
      std::cout << it->first << "\n"; // print filename
      for (auto &&msg : it->second) {
        msg.Display();
      }
      std::cout << "\n";
    }
  }

  if (!has_fatal_errors) {
    Accept(node);
    return true;
  } else {
    return false;
  }
}

void Compiler::Accept(AstModule *node) {
  for (int i = 0; i < node->children.size(); i++) {
    Accept(node->children[i].get());
  }
}

void Compiler::Accept(AstNode *node) {
  if (!node) {
    return;
  }

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
    break;
  }
}

void Compiler::Accept(AstImports *node) {
  for (auto &child : node->children) {
    Accept(child.get());
  }
}

void Compiler::Accept(AstImport *node) {
  // module is already loaded at this point
  std::string path = node->relative_path + node->import_str;
  auto it = state.other_modules.find(path);
  if (it != state.other_modules.end()) {
    for (auto &&child : it->second->children) {
      Accept(child.get());
    }
  }
}

void Compiler::Accept(AstStatement *node) {
}

void Compiler::Accept(AstBlock *node) {
  size_t counter = 0;

  while (counter < node->children.size()) {
    auto &child = node->children[counter];
    Accept(child.get());

    if (config::optimize_remove_dead_code && child->type == ast_type_return) {
      counter = node->children.size();
      break;
    }

    ++counter;
  }
}

void Compiler::Accept(AstExpression *node) {
  if (config::optimize_constant_folding) {
    OptimizeAstNode(node->child);
  }
  Accept(node->child.get());

  if (node->should_clear_stack) {
    bstream << Instruction<OpCode_t>(OpCode_pop);
  }
}

void Compiler::Accept(AstBinaryOp *node) {
  auto &left = node->left;
  auto &right = node->right;

  if (node->op == BinOp_greater) {
    /* reverse placement of operands:
        a > b will now be b < a */
    Accept(right.get());
    Accept(left.get());
    bstream << Instruction<OpCode_t>(OpCode_less);
  } else if (node->op == BinOp_greater_eql) {
    /* reverse placement of operands:
    a >= b will now be b <= a */
    Accept(right.get());
    Accept(left.get());
    bstream << Instruction<OpCode_t>(OpCode_less_eql);
  } else {
    Accept(left.get());
    Accept(right.get());

    switch (node->op) {
    case BinOp_power:
      bstream << Instruction<OpCode_t>(OpCode_pow);
      break;
    case BinOp_multiply:
      bstream << Instruction<OpCode_t>(OpCode_mul);
      break;
    case BinOp_floor_divide:
    case BinOp_divide:
      bstream << Instruction<OpCode_t>(OpCode_div);
      break;
    case BinOp_modulus:
      bstream << Instruction<OpCode_t>(OpCode_mod);
      break;
    case BinOp_add:
      bstream << Instruction<OpCode_t>(OpCode_add);
      break;
    case BinOp_subtract:
      bstream << Instruction<OpCode_t>(OpCode_sub);
      break;
    case BinOp_logand:
      bstream << Instruction<OpCode_t>(OpCode_and);
      break;
    case BinOp_logor:
      bstream << Instruction<OpCode_t>(OpCode_or);
      break;
    case BinOp_equals:
      bstream << Instruction<OpCode_t>(OpCode_eql);
      break;
    case BinOp_not_equal:
      bstream << Instruction<OpCode_t>(OpCode_neql);
      break;
    case BinOp_less:
      bstream << Instruction<OpCode_t>(OpCode_less);
      break;
    case BinOp_less_eql:
      bstream << Instruction<OpCode_t>(OpCode_less_eql);
      break;
    case BinOp_bitand:
      bstream << Instruction<OpCode_t>(OpCode_bit_and);
      break;
    case BinOp_bitor:
      bstream << Instruction<OpCode_t>(OpCode_bit_or);
      break;
    case BinOp_bitxor:
      bstream << Instruction<OpCode_t>(OpCode_bit_xor);
      break;
    case BinOp_bitshift_left:
      bstream << Instruction<OpCode_t>(OpCode_left_shift);
      break;
    case BinOp_bitshift_right:
      bstream << Instruction<OpCode_t>(OpCode_right_shift);
      break;
    case BinOp_assign:
      bstream << Instruction<OpCode_t>(OpCode_assign);
      break;
    case BinOp_add_assign:
      bstream << Instruction<OpCode_t>(OpCode_add_assign);
      break;
    case BinOp_subtract_assign:
      bstream << Instruction<OpCode_t>(OpCode_sub_assign);
      break;
    case BinOp_multiply_assign:
      bstream << Instruction<OpCode_t>(OpCode_mul_assign);
      break;
    case BinOp_divide_assign:
      bstream << Instruction<OpCode_t>(OpCode_div_assign);
      break;
    }
  }
}

void Compiler::Accept(AstUnaryOp *node) {
  Accept(node->child.get());

  switch (node->op) {
  case UnOp_lognot:
    bstream << Instruction<OpCode_t>(OpCode_unary_not);
    break;
  case UnOp_negative:
    bstream << Instruction<OpCode_t>(OpCode_unary_minus);
    break;
  }
}

void Compiler::Accept(AstArrayAccess *node) {
  // push the value
  // push the index
  // push the instruction
  Accept(node->object.get());
  Accept(node->index.get());
  bstream << Instruction<OpCode_t>(OpCode_array_index);
}

void Compiler::Accept(AstMemberAccess *node) {
  // checks for module with name first
  AstModule *found_module = nullptr;
  if (state.FindModule(node->left_str, node->module, found_module)) {
    // it is a module being referenced
    // set the right node's module to be the one we found
    node->right->module = found_module;
    Accept(node->right.get());
  } else {
    Accept(node->left.get());
    if (node->right->type == ast_type_member_access) {
      Accept(node->right.get());
      auto right_ast = static_cast<AstMemberAccess*>(node->right.get());
      bstream << Instruction<OpCode_t, int32_t, const char *>(OpCode_load_member, right_ast->left_str.length() + 1, right_ast->left_str.c_str());
    } else if (node->right->type == ast_type_variable) {
      auto right_ast = static_cast<AstVariable*>(node->right.get());
      bstream << Instruction<OpCode_t, int32_t, const char *>(OpCode_load_member, right_ast->name.length() + 1, right_ast->name.c_str());
    } else if (node->right->type == ast_type_function_call) {
      // accept member function call
      auto right_ast = static_cast<AstFunctionCall*>(node->right.get());

      for (auto &&param : right_ast->arguments) {
        Accept(param.get());
      }

      bstream << Instruction<OpCode_t, int32_t, const char *>(OpCode_load_member, right_ast->name.length() + 1, right_ast->name.c_str());
      bstream << Instruction<OpCode_t, int32_t>(OpCode_invoke_object, right_ast->arguments.size());
    }
  }
}

void Compiler::Accept(AstModuleAccess *node) {
  for (auto &&mod : state.other_modules) {
    if (mod.second->name == node->module_name) {
      node->right->module = mod.second.get();
      Accept(node->right.get());
      return;
    }
  }
}

void Compiler::Accept(AstVariableDeclaration *node) {
  Accept(node->assignment.get()); // may have side effects, so accept anyway
  if ((config::optimize_remove_unused && UseCount(node) != 0) || !config::optimize_remove_unused) {
    std::string var_name(state.MakeVariableName(node->name, node->module));
    bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_store_as_local, var_name.length() + 1, var_name.c_str());
  } else {
    // must pop the result of the assignment from the stack
    bstream << Instruction<OpCode_t>(OpCode_pop);
  }
}

void Compiler::Accept(AstAlias *node) {
  // do nothing
}

void Compiler::Accept(AstUseModule *node) {
  // do nothing
}

void Compiler::Accept(AstVariable *node) {
  if (node->is_alias) {
    Accept(node->alias_to);
  } else {
    if (config::optimize_constant_folding && node->is_const && node->is_literal && node->current_value != nullptr) {
      Accept(node->current_value);
    } else {
      std::string var_name(state.MakeVariableName(node->name, node->module));
      bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_load_local,
        var_name.length() + 1, var_name.c_str());
    }
  }
}

void Compiler::Accept(AstInteger *node) {
  bstream << Instruction<OpCode_t, AVMInteger_t>(OpCode_load_integer, node->value);
}

void Compiler::Accept(AstFloat *node) {
  bstream << Instruction<OpCode_t, AVMFloat_t>(OpCode_load_float, node->value);
}

void Compiler::Accept(AstString *node) {
  bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_load_string,
    node->value.length() + 1, node->value.c_str());
}

void Compiler::Accept(AstTrue *node) {
  bstream << Instruction<OpCode_t, AVMInteger_t>(OpCode_load_integer, 1);
}

void Compiler::Accept(AstFalse *node) {
  bstream << Instruction<OpCode_t, AVMInteger_t>(OpCode_load_integer, 0);
}

void Compiler::Accept(AstNull *node) {
  bstream << Instruction<OpCode_t>(OpCode_load_null);
}

void Compiler::Accept(AstSelf *node) {
}

void Compiler::Accept(AstNew *node) {
}

void Compiler::Accept(AstFunctionDefinition *node) {
  if ((config::optimize_remove_unused && UseCount(node) != 0) || !config::optimize_remove_unused) {
    if (!node->HasAttribute("inline")) {
      bool is_global_function = state.function_level == 0;
      ++state.function_level;

      std::string var_name(state.MakeVariableName(node->name, node->module));

      bstream << Instruction<OpCode_t, uint8_t, uint32_t, uint8_t>(OpCode_new_function,
        static_cast<uint8_t>(is_global_function), node->arguments.size(), 0/*No variadic support yet*/);
      bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_store_as_local, var_name.length() + 1, var_name.c_str());

      auto *body = dynamic_cast<AstBlock*>(node->block.get());
      if (body) {
        IncreaseBlock(LevelType::Level_function);

        // create params as local variables
        for (auto it = node->arguments.rbegin(); it != node->arguments.rend(); ++it) {
          std::string arg_name = state.MakeVariableName(*it, node->module);
          bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_store_as_local, arg_name.length() + 1, arg_name.c_str());
        }

        Accept(body);
        DecreaseBlock();

        // Return instruction is placed after the block is decreased,
        // so that the function will finish execution of the block before ending.
        bstream << Instruction<OpCode_t>(OpCode_return);
      }

      --state.function_level;
    }
  }
}

void Compiler::Accept(AstFunctionExpression *node) {
  bstream << Instruction<OpCode_t, uint8_t, uint32_t, uint8_t>(OpCode_new_function, 0,
    node->arguments.size(), 0/*No variadic support yet*/);

  auto *body = dynamic_cast<AstBlock*>(node->block.get());
  if (body) {
    IncreaseBlock(LevelType::Level_function);

    // create params as local variables
    for (auto it = node->arguments.rbegin(); it != node->arguments.rend(); ++it) {
      std::string var_name = state.MakeVariableName(*it, node->module);
      bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_store_as_local,
        var_name.length() + 1, var_name.c_str());
    }

    Accept(body);
    DecreaseBlock();

    // Return instruction is placed after the block is decreased,
    // so that the function will finish execution of the block before ending.
    bstream << Instruction<OpCode_t>(OpCode_return);
  }
}

void Compiler::Accept(AstFunctionCall *node) {
  // Push each argument onto the stack
  for (auto &&param : node->arguments) {
    Accept(param.get());
  }

  if (node->is_alias) {
    Accept(node->alias_to);
  } else {
    // check if we can inline this function
    bool inlined = false;
    if (node->definition != nullptr) {
      auto *def = dynamic_cast<AstFunctionDefinition*>(node->definition);
      if (def != nullptr) {
        if (def->HasAttribute("inline")) {
          ++state.function_level;
          bstream << Instruction<OpCode_t>(OpCode_irl);
          IncreaseBlock(LevelType::Level_function);

          // create params as local variables
          for (auto it = def->arguments.rbegin(); it != def->arguments.rend(); ++it) {
            std::string arg_name = state.MakeVariableName(*it, def->module);
            bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_store_as_local,
              arg_name.length() + 1, arg_name.c_str());
          }

          Accept(def->block.get());
          DecreaseBlock();
          --state.function_level;
          inlined = true;
        }
      }
    }

    if (!inlined) {
      std::string var_name = state.MakeVariableName(node->name, node->module);
      bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_load_local,
        var_name.length() + 1, var_name.c_str());
    }
  }

  bstream << Instruction<OpCode_t, int32_t>(OpCode_invoke_object, node->arguments.size());
}

void Compiler::Accept(AstClass *node) {
}

void Compiler::Accept(AstObjectExpression *node) {
  bstream << Instruction<OpCode_t>(OpCode_new_structure);
  for (auto &&mem : node->members) {
    bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_new_member, mem.first.length() + 1, mem.first.c_str());
    Accept(mem.second.get());
    bstream << Instruction<OpCode_t>(OpCode_assign);
    bstream << Instruction<OpCode_t>(OpCode_pop);
  }
  // the structure remains on the stack
}

void Compiler::Accept(AstEnum *node) {
}

void Compiler::Accept(AstIfStmt *node) {
  Accept(node->conditional.get());
  bstream << Instruction<OpCode_t>(OpCode_irl_if_true);

  IncreaseBlock(LevelType::Level_condition);
  Accept(node->block.get());
  DecreaseBlock();

  if (node->else_statement) {
    bstream << Instruction<OpCode_t>(OpCode_irl_if_false);
    IncreaseBlock(LevelType::Level_condition);
    Accept(node->else_statement.get());
    DecreaseBlock();
  }
}

void Compiler::Accept(AstPrintStmt *node) {
  // loop through in reverse order
  for (auto it = node->arguments.rbegin(); it != node->arguments.rend(); ++it) {
    Accept(it->get());
  }
  bstream << Instruction<OpCode_t, uint32_t>(OpCode_print, node->arguments.size());
}

void Compiler::Accept(AstReturnStmt *node) {
  // The resulting value will get pushed onto the stack
  Accept(node->value.get());

  int start = state.level;
  int counter = 1;
  LevelInfo *level = &state.levels[start];
  while (start >= compiler_global_level && level->type != LevelType::Level_function) {
    ++counter;
    level = &state.levels[--start];
  }
  bstream << Instruction<OpCode_t, uint8_t>(OpCode_drl, counter);
}

void Compiler::Accept(AstForLoop *node) {
  /* bstream << Instruction<OpCode_t>(OpCode_irl);
   IncreaseBlock(LevelType::Level_default);

   auto id = ++state.block_id_counter;

   if (node->range->type == ast_type_range) {
       AstRange *range = static_cast<AstRange*>(node->range.get());
       AVMInteger_t first = range->first, second = range->second;
       AVMInteger_t diff = second - first;
       bool pos = diff >= 0;

       std::string var_name = state.MakeVariableName(node->identifier, node->module);
       bstream << Instruction<OpCode_t, int32_t, const char*>(OpCode_load_local,
           var_name.length() + 1, var_name.c_str());

       bstream << Instruction<OpCode_t, AVMInteger_t>(OpCode_load_integer, first);
       bstream << Instruction<OpCode_t>(OpCode_assign);

       bstream << Instruction<OpCode_t, AVMInteger_t>(OpCode_load_integer, second);
       bstream << Instruction<OpCode_t>(OpCode_neql);
       bstream << Instruction<OpCode_t>(OpCode_irl_if_true);

       IncreaseBlock(LevelType::Level_loop);
       Accept(node->block.get());
       DecreaseBlock();

       bstream << Instruction<OpCode_t, AVMInteger_t>(OpCode_load_integer, first);
       bstream << Instruction<OpCode_t, AVMInteger_t>(OpCode_add, pos ? 1 : -1);
   }

   bstream << Instruction<OpCode_t, uint32_t>(OpCode_jump_if_true, id);
   DecreaseBlock();*/
  bool empty_body = true;
  if (node->block != nullptr) {
    AstBlock *block = dynamic_cast<AstBlock*>(node->block.get());
    if (block && !block->children.empty()) {
      empty_body = false;
    }
  }

  if ((config::optimize_remove_dead_code && !empty_body) || !config::optimize_remove_dead_code) {
    auto id = ++state.block_id_counter;

    bstream << Instruction<OpCode_t>(OpCode_irl);
    IncreaseBlock(LevelType::Level_default);
    Accept(node->initializer.get());
    bstream << Instruction<OpCode_t, uint32_t>(OpCode_store_address, id);

    Accept(node->conditional.get());
    bstream << Instruction<OpCode_t>(OpCode_irl_if_true);

    IncreaseBlock(LevelType::Level_loop);
    Accept(node->block.get());
    DecreaseBlock();
    Accept(node->afterthought.get());

    bstream << Instruction<OpCode_t, uint32_t>(OpCode_jump_if_true, id);
    DecreaseBlock();
  }
}

void Compiler::Accept(AstWhileLoop *node) {
  bool empty_body = true;
  if (node->block != nullptr) {
    AstBlock *block = dynamic_cast<AstBlock*>(node->block.get());
    if (block && !block->children.empty()) {
      empty_body = false;
    }
  }

  if ((config::optimize_remove_dead_code && !empty_body) || !config::optimize_remove_dead_code) {
    auto id = ++state.block_id_counter;

    bstream << Instruction<OpCode_t, uint32_t>(OpCode_store_address, id);

    Accept(node->conditional.get());
    bstream << Instruction<OpCode_t>(OpCode_irl_if_true);

    IncreaseBlock(LevelType::Level_loop);
    Accept(node->block.get());
    DecreaseBlock();

    bstream << Instruction<OpCode_t, uint32_t>(OpCode_jump_if_true, id);
  }
}

void Compiler::Accept(AstTryCatch *node) {
  // First check that the try block is not empty
  bool empty_try_body = true;
  if (node->try_block != nullptr) {
    AstBlock *block = dynamic_cast<AstBlock*>(node->try_block.get());
    if (block && !block->children.empty()) {
      empty_try_body = false;
    }
  }

  if ((config::optimize_remove_dead_code && !empty_try_body) || !config::optimize_remove_dead_code) {
    bstream << Instruction<OpCode_t>(OpCode_try_catch_block);

    IncreaseBlock(LevelType::Level_default);
    Accept(node->try_block.get());
    DecreaseBlock();

    IncreaseBlock(LevelType::Level_default);
    Accept(node->exception_object.get());
    Accept(node->catch_block.get());
    DecreaseBlock();
  }
}

void Compiler::Accept(AstRange *node) {
}

size_t Compiler::UseCount(AstNode *node) {
  if (state.use_counts.find(node) == state.use_counts.end())
    return 0;
  else
    return state.use_counts[node];
}

void Compiler::IncreaseBlock(LevelType type) {
  LevelInfo level;
  level.type = type;
  state.levels[++state.level] = level;
  bstream << Instruction<OpCode_t>(OpCode_ifl);
}

void Compiler::DecreaseBlock() {
  state.levels[state.level--] = LevelInfo();
  bstream << Instruction<OpCode_t>(OpCode_dfl);
}

void Compiler::OptimizeAstNode(std::unique_ptr<AstNode> &node) {
  auto optimized = node->Optimize();
  if (optimized != nullptr) {
    node.swap(optimized);
  } 
}
} // namespace avm