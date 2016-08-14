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
    std::unique_ptr<AstNode> swapped(optimize(std::move(node->child)));
    node->child.swap(swapped);
  }
  Accept(node->child.get());

  if (node->should_clear_stack) {
    bstream << Instruction<Opcode_t>(ins_pop);
  }
}

void Compiler::Accept(AstBinaryOp *node) {
  auto &left = node->left;
  auto &right = node->right;

  if (node->op == opr_greater) {
    /* reverse placement of operands:
        a > b will now be b < a */
    Accept(right.get());
    Accept(left.get());
    bstream << Instruction<Opcode_t>(ins_less);
  } else if (node->op == opr_greater_eql) {
    /* reverse placement of operands:
    a >= b will now be b <= a */
    Accept(right.get());
    Accept(left.get());
    bstream << Instruction<Opcode_t>(ins_less_eql);
  } else {
    Accept(left.get());
    Accept(right.get());

    switch (node->op) {
    case opr_power:
      bstream << Instruction<Opcode_t>(ins_pow);
      break;
    case opr_multiply:
      bstream << Instruction<Opcode_t>(ins_mul);
      break;
    case opr_floor_divide:
    case opr_divide:
      bstream << Instruction<Opcode_t>(ins_div);
      break;
    case opr_modulus:
      bstream << Instruction<Opcode_t>(ins_mod);
      break;
    case opr_add:
      bstream << Instruction<Opcode_t>(ins_add);
      break;
    case opr_subtract:
      bstream << Instruction<Opcode_t>(ins_sub);
      break;
    case opr_logand:
      bstream << Instruction<Opcode_t>(ins_and);
      break;
    case opr_logor:
      bstream << Instruction<Opcode_t>(ins_or);
      break;
    case opr_equals:
      bstream << Instruction<Opcode_t>(ins_eql);
      break;
    case opr_nequal:
      bstream << Instruction<Opcode_t>(ins_neql);
      break;
    case opr_less:
      bstream << Instruction<Opcode_t>(ins_less);
      break;
    case opr_less_eql:
      bstream << Instruction<Opcode_t>(ins_less_eql);
      break;
    case opr_bitand:
      bstream << Instruction<Opcode_t>(ins_bit_and);
      break;
    case opr_bitor:
      bstream << Instruction<Opcode_t>(ins_bit_or);
      break;
    case opr_bitxor:
      bstream << Instruction<Opcode_t>(ins_bit_xor);
      break;
    case opr_bitshift_left:
      bstream << Instruction<Opcode_t>(ins_left_shift);
      break;
    case opr_bitshift_right:
      bstream << Instruction<Opcode_t>(ins_right_shift);
      break;
    case opr_assignment:
      bstream << Instruction<Opcode_t>(ins_assign);
      break;
    case opr_add_assign:
      bstream << Instruction<Opcode_t>(ins_add_assign);
      break;
    case opr_subtract_assign:
      bstream << Instruction<Opcode_t>(ins_sub_assign);
      break;
    case opr_multiply_assign:
      bstream << Instruction<Opcode_t>(ins_mul_assign);
      break;
    case opr_divide_assign:
      bstream << Instruction<Opcode_t>(ins_div_assign);
      break;
    }
  }
}

void Compiler::Accept(AstUnaryOp *node) {
  Accept(node->child.get());

  switch (node->op) {
  case opr_lognot:
    bstream << Instruction<Opcode_t>(ins_unary_not);
    break;
  case opr_negative:
    bstream << Instruction<Opcode_t>(ins_unary_minus);
    break;
  }
}

void Compiler::Accept(AstArrayAccess *node) {
  // push the value
  // push the index
  // push the instruction
  Accept(node->object.get());
  Accept(node->index.get());
  bstream << Instruction<Opcode_t>(ins_array_index);
}

void Compiler::Accept(AstMemberAccess *node) {
  // checks for module with name first
  AstModule *found_module = nullptr;
  if (state.FindModule(node->left_str, node->module, found_module)) {
    // it is a module being referenced
    // set the right node's module to be the one we found
    node->right->module = found_module;
    Accept(node->right.get());
  }  // todo variable member access
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
  if ((config::optimize_remove_unused && UseCount(node) != 0) || !config::optimize_remove_unused) {
    Accept(node->assignment.get());
    std::string var_name(state.MakeVariableName(node->name, node->module));
    bstream << Instruction<Opcode_t, int32_t, const char*>(ins_store_as_local, var_name.length() + 1, var_name.c_str());
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
    std::string var_name(state.MakeVariableName(node->name, node->module));
    bstream << Instruction<Opcode_t, int32_t, const char*>(ins_load_local, 
      var_name.length() + 1, var_name.c_str());
  }
}

void Compiler::Accept(AstInteger *node) {
  bstream << Instruction<Opcode_t, AVMInteger_t>(ins_load_integer, node->value);
}

void Compiler::Accept(AstFloat *node) {
  bstream << Instruction<Opcode_t, AVMFloat_t>(ins_load_float, node->value);
}

void Compiler::Accept(AstString *node) {
  bstream << Instruction<Opcode_t, int32_t, const char*>(ins_load_string, 
    node->value.length() + 1, node->value.c_str());
}

void Compiler::Accept(AstTrue *node) {
  bstream << Instruction<Opcode_t, AVMInteger_t>(ins_load_integer, 1);
}

void Compiler::Accept(AstFalse *node) {
  bstream << Instruction<Opcode_t, AVMInteger_t>(ins_load_integer, 0);
}

void Compiler::Accept(AstNull *node) {
  bstream << Instruction<Opcode_t>(ins_load_null);
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

      bstream << Instruction<Opcode_t, uint8_t, uint32_t, uint8_t>(ins_new_function,
        static_cast<uint8_t>(is_global_function), node->arguments.size(), 0/*No variadic support yet*/);
      bstream << Instruction<Opcode_t, int32_t, const char*>(ins_store_as_local, var_name.length() + 1, var_name.c_str());

      auto *body = dynamic_cast<AstBlock*>(node->block.get());
      if (body) {
        IncreaseBlock(LevelType::LEVEL_FUNCTION);

        // create params as local variables
        for (auto it = node->arguments.rbegin(); it != node->arguments.rend(); ++it) {
          std::string arg_name = state.MakeVariableName(*it, node->module);
          bstream << Instruction<Opcode_t, int32_t, const char*>(ins_store_as_local, arg_name.length() + 1, arg_name.c_str());
        }

        Accept(body);
        DecreaseBlock();

        bstream << Instruction<Opcode_t>(ins_return);
      }

      --state.function_level;
    }
  }
}

void Compiler::Accept(AstFunctionExpression *node) {
  bstream << Instruction<Opcode_t, uint8_t, uint32_t, uint8_t>(ins_new_function, 0, 
    node->arguments.size(), 0/*No variadic support yet*/);

  auto *body = dynamic_cast<AstBlock*>(node->block.get());
  if (body) {
    IncreaseBlock(LevelType::LEVEL_FUNCTION);

    // create params as local variables
    for (auto it = node->arguments.rbegin(); it != node->arguments.rend(); ++it) {
      std::string var_name = state.MakeVariableName(*it, node->module);
      bstream << Instruction<Opcode_t, int32_t, const char*>(ins_store_as_local, 
        var_name.length() + 1, var_name.c_str());
    }

    Accept(body);
    DecreaseBlock();

    bstream << Instruction<Opcode_t>(ins_return);
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
          bstream << Instruction<Opcode_t>(ins_irl);
          IncreaseBlock(LevelType::LEVEL_FUNCTION);

          // create params as local variables
          for (auto it = def->arguments.rbegin(); it != def->arguments.rend(); ++it) {
            std::string arg_name = state.MakeVariableName(*it, def->module);
            bstream << Instruction<Opcode_t, int32_t, const char*>(ins_store_as_local, 
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
      bstream << Instruction<Opcode_t, int32_t, const char*>(ins_load_local, 
        var_name.length() + 1, var_name.c_str());

      bstream << Instruction<Opcode_t, int32_t>(ins_invoke_object, node->arguments.size());
    }
  }
}

void Compiler::Accept(AstClass *node) {
}

void Compiler::Accept(AstEnum *node) {
}

void Compiler::Accept(AstIfStmt *node) {
  Accept(node->conditional.get());
  bstream << Instruction<Opcode_t>(ins_irl_if_true);

  IncreaseBlock(LevelType::LEVEL_CONDITIONAL);
  Accept(node->block.get());
  DecreaseBlock();

  if (node->else_statement) {
    bstream << Instruction<Opcode_t>(ins_irl_if_false);
    IncreaseBlock(LevelType::LEVEL_CONDITIONAL);
    Accept(node->else_statement.get());
    DecreaseBlock();
  }
}

void Compiler::Accept(AstPrintStmt *node) {
  // loop through in reverse order
  for (auto it = node->arguments.rbegin(); it != node->arguments.rend(); ++it) {
    Accept(it->get());
  }
  bstream << Instruction<Opcode_t, uint32_t>(ins_print, node->arguments.size());
}

void Compiler::Accept(AstReturnStmt *node) {
  // The resulting value will get pushed onto the stack
  Accept(node->value.get());

  int start = state.level;
  int counter = 1;
  LevelInfo *level = &state.levels[start];

  while (start >= compiler_global_level && level->type != LevelType::LEVEL_FUNCTION) {
    ++counter;
    level = &state.levels[--start];
  }
  bstream << Instruction<Opcode_t, uint8_t>(ins_drl, counter);
}

void Compiler::Accept(AstForLoop *node) {
  bool empty_body = true;
  if (node->block != nullptr) {
    AstBlock *block = dynamic_cast<AstBlock*>(node->block.get());
    if (block && !block->children.empty()) {
      empty_body = false;
    }
  }

  if ((config::optimize_remove_dead_code && !empty_body) || !config::optimize_remove_dead_code) {
    auto id = ++state.block_id_counter;

    bstream << Instruction<Opcode_t>(ins_irl);
    IncreaseBlock(LevelType::LEVEL_DEFAULT);
    Accept(node->initializer.get());
    bstream << Instruction<Opcode_t, uint32_t>(ins_store_address, id);

    Accept(node->conditional.get());
    bstream << Instruction<Opcode_t>(ins_irl_if_true);

    IncreaseBlock(LevelType::LEVEL_LOOP);
    Accept(node->block.get());
    DecreaseBlock();
    Accept(node->afterthought.get());

    bstream << Instruction<Opcode_t, uint32_t>(ins_jump_if_true, id);
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

    bstream << Instruction<Opcode_t, uint32_t>(ins_store_address, id);

    Accept(node->conditional.get());
    bstream << Instruction<Opcode_t>(ins_irl_if_true);

    IncreaseBlock(LevelType::LEVEL_LOOP);
    Accept(node->block.get());
    DecreaseBlock();

    bstream << Instruction<Opcode_t, uint32_t>(ins_jump_if_true, id);
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
    bstream << Instruction<Opcode_t>(ins_try_catch_block);

    IncreaseBlock(LevelType::LEVEL_DEFAULT);
    Accept(node->try_block.get());
    DecreaseBlock();

    IncreaseBlock(LevelType::LEVEL_DEFAULT);
    Accept(node->exception_object.get());
    Accept(node->catch_block.get());
    DecreaseBlock();
  }
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
  bstream << Instruction<Opcode_t>(ins_ifl);
}

void Compiler::DecreaseBlock() {
  state.levels[state.level--] = LevelInfo();
  bstream << Instruction<Opcode_t>(ins_dfl);
}
} // namespace avm