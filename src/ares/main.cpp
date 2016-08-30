#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iomanip>

#include <ascript.h>

int main(int argc, char *argv[]) {
  std::string code = "";
  std::string output_file = "";
  std::string input_file = "";
  bool code_loaded = false;

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      if (i + 1 != argc) {
        if (std::strcmp(argv[i], "-o") == 0) {
          // next should be the output path
          output_file = argv[i + 1];
        } else if (std::strcmp(argv[i], "-code") == 0) {
          code = argv[i + 1];
          code_loaded = true;
        }
      }
    }

    if (!code_loaded) {
      input_file = argv[1];
      std::ifstream file(input_file);

      if (!file.is_open()) {
        std::cout << "File not found: " << input_file << "\n";
        return 1;
      }

      code = std::string((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
      code_loaded = true;

      file.close();
    }

    if (output_file.empty() && !input_file.empty()) {
      size_t last_index = input_file.find_last_of(".");
      output_file = input_file.substr(0, last_index) + ".ac";
    }

    ares::Script script(code, input_file, output_file);
    if (!script.Run()) {
      system("pause");
      return 1;
    }

  } else {
    std::string program_path(argv[0]);
    std::string program_file =
      program_path.substr(program_path.find_last_of("/\\") + 1);

    std::cout << "Usage: " << program_file << " <filepath>\n";
    std::cout << "\t-o <filepath>: Output bytecode to a specified file.\n";
    std::cout << "\t-code <code string>: Execute code from a string, rather than from a file.\n";
  }
  return 0;
}