#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>
#include <utility>
#include <iostream>

#include <detail/location.h>
#include <detail/token.h>
#include <detail/error.h>
#include <detail/ast.h>

namespace avm {
static const int compiler_global_level = 0;

struct Symbol {
  // Pointer to ast node (declaration)
  AstNode *node = nullptr;
  // Original name before being mangled
  std::string original_name;
  // Is this an alias for another symbol?
  bool is_alias = false;
  // Is this a constant? (immutable)
  bool is_const = false;
  // Is this currently set to a literal?
  bool is_literal = false;
  // Current value (used when set to literal)
  AstNode *current_value = nullptr;
  // If it is an alias, it is an alias to what?
  std::string alias_to;
  // Is is a natively defined object?
  bool is_native = false;
  // Number of parameters required, if it is a native function
  size_t nargs = 0;
};

struct CompilerState {
  enum LevelType {
    Level_default,
    Level_function,
    Level_loop,
    Level_condition
  };

  struct ExternalFunction {
    std::string name;
    size_t nargs;
  };

  struct ModuleDefine {
    std::string name;

    ModuleDefine &Define(const std::string &name, size_t nargs) {
      methods.push_back({ name, nargs });
      return *this;
    }

    std::vector<ExternalFunction> methods;
  };

  struct LevelInfo {
    LevelType type;
    std::vector<
      std::pair<
      std::string,
      Symbol
      >
    > locals;
  };

  std::vector<BuildMessage> errors;
  std::map<AstNode*, size_t> use_counts;
  std::vector<AstFunctionCall*> native_function_calls;

  std::map<
    std::string,
    std::unique_ptr<
    AstModule
    >
  > other_modules;

  std::map<int, LevelInfo> levels;
  int level, function_level /* function nesting level */;
  unsigned int block_id_counter;

  CompilerState() {
    block_id_counter = 0;
    level = compiler_global_level;
    function_level = 0;
    levels[level] = LevelInfo();
  }

  CompilerState(CompilerState &&other) {
    errors = other.errors;
    use_counts = other.use_counts;
    native_function_calls = other.native_function_calls;
    levels = other.levels;
    level = other.level;
    function_level = other.function_level;
    block_id_counter = other.block_id_counter;

    for (auto &&it : other.other_modules) {
      typedef std::pair<
        std::string,
        std::unique_ptr<
        AstModule
        >
      > module_pair;

      other_modules.insert(module_pair(it.first, std::move(it.second)));
    }
  }

  // Mangles the variable name to avoid name clashing.
  std::string MakeVariableName(const std::string &original, AstNode *module) {
    auto *module_cast = dynamic_cast<AstModule*>(module);
    if (!module_cast) {
      throw std::runtime_error("bad module");
    }
    std::string module_name = module_cast->name;
    std::string mangled_name = module_name + "_" + original;
    return mangled_name;
  }

  // Returns true if the module is imported. ('module' is the current module).
  bool FindModule(const std::string &name, AstNode *module = nullptr) {
    AstModule *tmp = nullptr;
    return FindModule(name, module, tmp);
  }

  // Returns true if module is imported; module will be stored in 'out'
  bool FindModule(const std::string &name, AstNode *module, AstModule *&out) {
    if (module != nullptr) {
      auto *module_ast = dynamic_cast<AstModule*>(module);
      if (!module_ast) {
        throw std::runtime_error("not of type AstModule");
      } else if (module_ast->name == name) {
        out = module_ast;
        return true;
      }
    }
    for (auto &&it : other_modules) {
      if (it.second->name == name) {
        out = it.second.get();
        return true;
      }
    }
    return false;
  }

  inline LevelInfo &current_level() {
    return levels[level];
  }
};

struct ParserState {
  int position = 0;

  std::vector<BuildMessage> errors;
  std::vector<Token> tokens;
};

struct LexerState {
  int position = 0;
  int length = 0;

  std::string source;
  std::string file;

  SourceLocation location;

  std::vector<BuildMessage> errors;
};
} // namespace avm

#endif