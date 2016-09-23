#include <ascript.h>
#include <rtlib.h>
#include <common/instructions.h>
#ifdef _MSC_VER
#include <platform/loadlib_windows.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iomanip>

bool IsBytecodeFile(const std::string &path)
{
    std::ifstream is(path, std::ios::in | std::ios::binary);
    // find the magic bytes
    // first check to see if the file is big enough
    is.seekg(0, std::ios::end);
    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);

    if (size < ARES_MAGIC_LEN + ARES_VERSION_LEN) {
        // file is not large enough to be valid
        is.close();
        return false;
    }

    // confirm that the magic bytes match
    char magic[ARES_MAGIC_LEN] = { '\0' };
    is.read((char*)&magic, ARES_MAGIC_LEN);
    if (!std::strncmp(magic, ARES_MAGIC, ARES_MAGIC_LEN)) {
        is.seekg(0, std::ios::beg);
        is.close();
        return true;
    }

    is.seekg(0, std::ios::beg);
    is.close();

    return false;
}

void Initialize()
{
#ifdef _MSC_VER
    ares::RuntimeLib::libloader = new ares::WindowsLibLoader();
#endif
}

void CleanUp()
{
    if (ares::RuntimeLib::libloader != nullptr) {
        delete ares::RuntimeLib::libloader;
        ares::RuntimeLib::libloader = nullptr;
    }
}

int main(int argc, char *argv[])
{
    Initialize();

    avm::Timer timer;
    timer.start();

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

            // read first few bytes to see if this is a bytecode file
            if (IsBytecodeFile(input_file)) {
                std::ifstream is(input_file, std::ios::binary | std::ios::ate);
                size_t max_pos = is.tellg();
                is.seekg(0);

                // set up buffer
                char *buffer = new char[max_pos];
                is.read(buffer, max_pos);
                is.close();

                // run compiled file
                ares::Script script;
                ares::ByteStream *stream = new ares::ByteStream(buffer, max_pos);

                script.RunFromBytecode(stream);
                CleanUp();

                delete stream; 
                delete[] buffer;

            } else {
                // assume it is a source code file
                std::ifstream file(input_file);
                if (!file.is_open()) {
                    std::cout << "File not found: " << input_file << "\n";
                    CleanUp();
                    return 1;
                }

                code = std::string((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
                code_loaded = true;

                file.close();

                // execute the code
                if (output_file.empty() && !input_file.empty()) {
                    size_t last_index = input_file.find_last_of(".");
                    output_file = input_file.substr(0, last_index) + ".ac";
                }

                ares::Script script;
                if (!script.CompileAndRun(code, input_file, output_file)) {
                    std::cin.get();
                    CleanUp();
                    return 1;
                }
            }
        }
    } else {
        std::string program_path(argv[0]);
        std::string program_file =
            program_path.substr(program_path.find_last_of("/\\") + 1);

        std::cout << "Usage: " << program_file << " <filepath>\n";
        std::cout << "\t-o <filepath>: Output bytecode to a specified file.\n";
        std::cout << "\t-code <code string>: Execute code from a string, rather than from a file.\n";
    }

    std::cout << "Elapsed time: " << timer.elapsed() << "\n";
    CleanUp();
    std::cin.get();
    return 0;
}