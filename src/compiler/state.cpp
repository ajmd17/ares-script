#include <detail/state.h>

namespace avm {
CompilerState::CompilerState()
    : block_id_counter(0),
    level(compiler_global_level),
    function_level(0)
{
    levels[level] = LevelInfo();
}

CompilerState::CompilerState(CompilerState &&other)
    : block_id_counter(other.block_id_counter),
    level(other.level),
    function_level(other.function_level),
    levels(other.levels),
    native_function_calls(other.native_function_calls),
    use_counts(other.use_counts),
    errors(other.errors)
{
    typedef std::pair<std::string, std::unique_ptr<AstModule>> ModuleStringPair;

    for (auto &&it : other.other_modules) {
        other_modules.insert(ModuleStringPair(it.first, std::move(it.second)));
    }
}

// Mangles the variable name to avoid name clashing.
std::string CompilerState::MakeVariableName(const std::string &original, AstNode *module)
{
    auto *module_cast = dynamic_cast<AstModule*>(module);
    if (!module_cast) {
        throw std::runtime_error("bad module");
    }
    std::string module_name = module_cast->name;
    std::string mangled_name = module_name + "_" + original;
    return mangled_name;
}

// Returns true if the module is imported. ('module' is the current module).
bool CompilerState::FindModule(const std::string &name, AstNode *module)
{
    AstModule *tmp = nullptr;
    return FindModule(name, module, tmp);
}

// Returns true if module is imported; module will be stored in 'out'
bool CompilerState::FindModule(const std::string &name, AstNode *module, AstModule *&out)
{
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
} // namespace avm