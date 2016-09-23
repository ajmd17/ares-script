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
    int owner_level = 0;
    int field_index = 0;

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
};

class SymbolTable {
public:
    SymbolTable()
    {
    }

    SymbolTable(SymbolTable &&other)
        : keys(std::move(other.keys)),
          values(std::move(other.values))
    {
    }

    inline size_t Size() const { return keys.size(); }

    inline void Insert(const std::string &key, std::unique_ptr<Symbol> &&value)
    {
        keys.push_back(key);
        values.push_back(std::move(value));
    }

    inline std::string &KeyAt(size_t index) { return keys.at(index); }
    inline const std::string &KeyAt(size_t index) const { return keys.at(index); }
    inline std::unique_ptr<Symbol> &ValueAt(size_t index) { return values.at(index); }
    inline const std::unique_ptr<Symbol> &ValueAt(size_t index) const { return values.at(index); }

    inline std::unique_ptr<Symbol> &At(const std::string &key)
    {
        auto key_it = std::find(keys.begin(), keys.end(), key);
        size_t key_index = key_it - keys.begin();
        return values.at(key_index);
    }

    inline const std::unique_ptr<Symbol> &At(const std::string &key) const
    {
        auto key_it = std::find(keys.begin(), keys.end(), key);
        size_t key_index = key_it - keys.begin();
        return values.at(key_index);
    }

private:
    std::vector<std::string> keys;
    std::vector<std::unique_ptr<Symbol>> values;
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
       /* SymbolTable locals;

        LevelInfo()
        {
        }

        LevelInfo(LevelInfo &&other)
            : type(std::move(other.type)),
              locals(std::move(other.locals))
        {
        }*/

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