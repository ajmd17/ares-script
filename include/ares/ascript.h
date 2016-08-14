#ifndef ASCRIPT_H
#define ASCRIPT_H

#include <string>
#include <fstream>

namespace ares {

/** Script Build Steps:
 *  Lexer
 *  - Analyzes text,
 *  - Returns a stream of tokens. 
 *  - Errors can be emitted for invalid characters
 *  Parser 
 *  - Accepts those tokens
 *  - Attempts to form a logical layout of the program (AST)
 *  - Errors can be emitted for poorly formed syntax
 *  Semantic Analyzer
 *  - Accepts the AST
 *  - Makes sure referenced identifiers are in place
 *  - Keeps track of how many times they are used.
 *  - Modifies the "compiler state" that is given to it
 *  - Errors can be emitted for undeclared identifiers
 *  Compiler
 *  - Accepts the AST (although with the modifed "compiler state")
 *  - Emits a stream of bytecode instructions
 *  - Does not emit errors per se, instead it will display all errors that have accumulated
 *  - Will refuse to continue if the error count is not zero
 *  Bytecode Generator
 *  - Accepts the stream of bytecode instructions
 *  - Begins to write them to the "filepath", supplied with the emit(filepath) function
 *  - Will stop writing if an unrecognized instruction is encountered (indicates an unimplemented feature)
 *  - The final stage of building the script and the file can be executed from "filepath" afterwords
*/
class Script {
public:
  Script(const std::string &code, const std::string &original_path, const std::string &output_file);
  ~Script();

  bool Run();

private:
  std::string code;
  std::string original_path;
  std::string output_file;
};
}

#endif