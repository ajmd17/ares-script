#include <ascript.h>
#include <rtlib.h>

#include <compiler/aparser.h>
#include <compiler/alexer.h>
#include <compiler/acompiler.h>
#include <compiler/abytecodegen.h>
#include <avm/avm.h>
#include <avm/detail/native_function.h>

#include <common/util/timer.h>

#include <fstream>
#include <iostream>

using namespace avm;
namespace ares {

static Timer global_timer = Timer();

Script::Script(const std::string &code, const std::string &original_path, const std::string &output_file) 
  : code(code), original_path(original_path), output_file(output_file) {
}

Script::~Script() {
}

double Tic() {
  global_timer.start();
  return 0;
}

double Toc() {
  return global_timer.elapsed();
}

/** \todo Make this check if the bytecode has already been generated, instead of re-compiling every time */
bool Script::Run() {
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
    compiler.Module("Reflection")
      .Define("typeof", 1);
    compiler.Module("Convert")
      .Define("toString", 1)
      .Define("toInt", 1)
      .Define("toBool", 1);
    compiler.Module("System")
      .Define("run", 1);
    compiler.Module("Console")
      .Define("write", 1)
      .Define("writeln", 1)
      .Define("readln", 0);

    if (compiler.Compile(unit.get())) {
      BytecodeGenerator gen(compiler.GetInstructions());
      ByteStream *stream = nullptr;

      std::ofstream file(output_file, std::ios::binary);
      if (!output_file.empty() && file.is_open()) {
        // output to bytecode file
        gen.Emit(file);
        file.close();

        stream = new FileByteStream(output_file);
      } else {
        // file did not open, so run from memory
        std::stringstream ss;
        gen.Emit(ss);

        stream = new MemoryByteStream(ss);
      }

      VMInstance *vm = new VMInstance();

      /*vm->BindFunction("FileIO_open", RuntimeLib::OpenFile);
      vm->BindFunction("FileIO_write", RuntimeLib::WriteStringToFile);
      vm->BindFunction("FileIO_read", RuntimeLib::ReadStringFromFile);
      vm->BindFunction("FileIO_close", RuntimeLib::CloseFile);

      vm->BindFunction("Clock_start", Tic);
      vm->BindFunction("Clock_stop", Toc);

      vm->BindFunction("Convert_toInt", RuntimeLib::ConvertToInt);
      vm->BindFunction("Convert_toBool", RuntimeLib::ConvertToBool);

      vm->BindFunction("System_run", RuntimeLib::SystemRun);

      vm->BindFunction("Console_write", RuntimeLib::ConsoleWrite);
      vm->BindFunction("Console_writeln", RuntimeLib::ConsoleWriteLn);
      vm->BindFunction("Console_readln", RuntimeLib::ConsoleReadLn);*/
      
      vm->BindFunction("Reflection_typeof", RuntimeLib::Reflection_typeof);

      vm->BindFunction("Convert_toString", RuntimeLib::Convert_toString);
      vm->BindFunction("Convert_toInt", RuntimeLib::Convert_toInt);
      vm->BindFunction("Convert_toBool", RuntimeLib::Convert_toBool);

      vm->Execute(stream);

      delete vm;
      delete stream;

      return true;
    } else {
      std::cout << "Compilation failed\n";
    }
  } else {
    std::cout << "Parsing failed\n";
  }

  return false;
}
} // namespace avm