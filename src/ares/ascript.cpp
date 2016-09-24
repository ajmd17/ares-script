#include <ascript.h>
#include <rtlib.h>

#include <compiler/parser.h>
#include <compiler/lexer.h>
#include <compiler/compiler.h>
#include <compiler/bytecode_generator.h>
#include <avm/avm.h>
#include <avm/detail/native_function.h>

#include <fstream>
#include <iostream>

using namespace avm;

namespace ares {
static Timer global_timer = Timer();

Script::Script()
{
}

Script::~Script()
{
}

void Tic(VMState *state, Object **args, uint32_t argc)
{
    if (CheckArgs(state, 0, argc)) {
        global_timer.start();

        auto ref = Reference(*state->heap.AllocObject<Variable>());
        ref.Ref()->flags |= Object::FLAG_TEMPORARY;
        state->stack.push_back(ref);
    }
}

void Toc(VMState *state, Object **args, uint32_t argc)
{
    if (CheckArgs(state, 0, argc)) {
        auto ref = Reference(*state->heap.AllocNull());
        auto result = new Variable();
        result->Assign(global_timer.elapsed());
        result->flags |= Object::FLAG_CONST;
        result->flags |= Object::FLAG_TEMPORARY;
        ref.Ref() = result;
        state->stack.push_back(ref);
    }
}

/** \todo Make this check if the bytecode has already been generated, instead of re-compiling every time */
bool Script::CompileAndRun(const std::string &code, 
    const std::string &original_path, const std::string &output_file)
{
    using namespace avm;

    Lexer lexer(code, original_path);
    auto tokens = lexer.ScanTokens();

    Parser parser(tokens, lexer.state);
    auto unit = parser.Parse();

    if (unit) {
        Compiler compiler(parser.state);

        compiler.Module("Clock")
            .Define("start", 0)
            .Define("stop", 0);
        compiler.Module("FileIO")
            .Define("open", 2)
            .Define("write", 2)
            .Define("read", 2)
            .Define("close", 1);
        compiler.Module("Runtime")
            .Define("loadlib", 1)
            .Define("loadfunc", 2)
            .Define("invoke", 1);
        compiler.Module("Reflection")
            .Define("typeof", 1);
        compiler.Module("Convert")
            .Define("toString", 1)
            .Define("toInt", 1)
            .Define("toFloat", 1)
            .Define("toBool", 1);
        compiler.Module("Console")
            .Define("system", 1)
            .Define("println", 1)
            .Define("readln", 0);

        if (compiler.Compile(unit.get())) {
            BytecodeGenerator gen(compiler.GetInstructions(), compiler.GetState().labels);
            
            char *buffer = nullptr;
            size_t max_pos = 0;

            std::ofstream file(output_file, std::ios::binary);
            if (!output_file.empty() && file.is_open()) {
                // output to bytecode file
                gen.Emit(file);
                file.close();

                std::ifstream is(output_file, std::ios::binary | std::ios::ate);

                max_pos = is.tellg();
                is.seekg(0);
                buffer = new char[max_pos];

                std::stringstream ss;
                ss << is.rdbuf();
                std::memcpy(buffer, ss.str().c_str(), max_pos);

            } else {
                // file did not open, so run from memory
                std::stringstream ss;
                gen.Emit(ss);

                max_pos = ss.tellp();
                buffer = new char[max_pos];
                std::memcpy(buffer, ss.str().c_str(), max_pos);
            }

            ByteStream *stream = new ByteStream(buffer, max_pos);
            RunFromBytecode(stream);
            delete stream;
            delete[] buffer;

            return true;
        } else {
            std::cout << "Compilation failed.\n";
        }
    } else {
        std::cout << "Parsing failed.\n";
    }

    return false;
}

void Script::RunFromBytecode(avm::ByteStream *stream)
{
    VMInstance *vm = new VMInstance();

    vm->BindFunction("Clock_start", Tic);
    vm->BindFunction("Clock_stop", Toc);

    vm->BindFunction("FileIO_open", RuntimeLib::FileIO_open);
    vm->BindFunction("FileIO_write", RuntimeLib::FileIO_write);
    vm->BindFunction("FileIO_read", RuntimeLib::FileIO_read);
    vm->BindFunction("FileIO_close", RuntimeLib::FileIO_close);

    vm->BindFunction("Runtime_loadlib", RuntimeLib::Runtime_loadlib);
    vm->BindFunction("Runtime_loadfunc", RuntimeLib::Runtime_loadfunc);
    vm->BindFunction("Runtime_invoke", RuntimeLib::Runtime_invoke);

    vm->BindFunction("Reflection_typeof", RuntimeLib::Reflection_typeof);

    vm->BindFunction("Convert_toString", RuntimeLib::Convert_toString);
    vm->BindFunction("Convert_toInt", RuntimeLib::Convert_toInt);
    vm->BindFunction("Convert_toFloat", RuntimeLib::Convert_toFloat);
    vm->BindFunction("Convert_toBool", RuntimeLib::Convert_toBool);

    vm->BindFunction("Console_system", RuntimeLib::Console_system);
    vm->BindFunction("Console_println", RuntimeLib::Console_println);
    vm->BindFunction("Console_readln", RuntimeLib::Console_readln);

    vm->Execute(stream);

    delete vm;
}
} // namespace avm
