#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <memory>

#include <detail/location.h>
#include <detail/token.h>
#include <detail/error.h>
#include <detail/ast.h>

namespace avm {
static const int compiler_global_level = 0;

struct SymbolQueryResult {
    bool found = false;
    Symbol *symbol = nullptr;

    inline operator bool() const { return found; }
};

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
    // the owner frame level of this symbol
    int owner_level = -1;
    // the index of this symbol
    int field_index = -1;
};

struct Label {
    unsigned int id = 0;
    unsigned int location = 0;
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

        inline ModuleDefine &Define(const std::string &name, size_t nargs)
        {
            methods.push_back({ name, nargs });
            return *this;
        }

        std::vector<ExternalFunction> methods;
    };

    struct LevelInfo {
        LevelType type;
        std::vector<std::pair<std::string, Symbol>> locals;
    };

    std::vector<BuildMessage> errors;
    std::map<AstNode*, size_t> use_counts;
    std::vector<AstFunctionCall*> native_function_calls;
    // a map of other imported modules
    std::map<std::string, std::unique_ptr<AstModule>> other_modules;

    // the key is the label id
    std::map<int, LevelInfo> levels;
    int level, function_level;
    std::vector<Label> labels;
    // the counter for levels
    unsigned int block_id_counter = 0;

    CompilerState();
    CompilerState(CompilerState &&other);

    // Mangles the variable name to avoid name clashing.
    std::string MakeVariableName(const std::string &original, AstNode *module);
    // Returns true if the module is imported. ('module' is the current module).
    bool FindModule(const std::string &name, AstNode *module = nullptr);
    // Returns true if module is imported; module will be stored in 'out'
    bool FindModule(const std::string &name, AstNode *module, AstModule *&out);

    inline LevelInfo &CurrentLevel() { return levels[level]; }
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