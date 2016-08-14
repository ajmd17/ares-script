#include <rtlib.h>
#include <iostream>
#include <cstdlib>

namespace avm {
FILE *RuntimeLib::OpenFile(const AVMString_t &filepath, const AVMString_t &mode) {
  return fopen(filepath.c_str(), mode.c_str());
}

AVMInteger_t RuntimeLib::WriteStringToFile(FILE *stream, const AVMString_t &str) {
  return fprintf(stream, str.c_str());
}

AVMString_t RuntimeLib::ReadStringFromFile(FILE *stream, AVMInteger_t len) {
  char *str = new char[len + 1];
  fgets(str, len, stream);
  AVMString_t astr(str);
  delete[] str;
  return astr;
}

AVMInteger_t RuntimeLib::CloseFile(FILE *stream) {
  return fclose(stream);
}

AVMInteger_t RuntimeLib::ConvertToInt(AVMFloat_t f) {
  return AVMInteger_t(f);
}

bool RuntimeLib::ConvertToBool(AVMInteger_t i) {
  return !!i;
}

AVMInteger_t RuntimeLib::SystemRun(const AVMString_t &str) {
  return std::system(str.c_str());
}

AVMString_t RuntimeLib::ConsoleWrite(const AVMString_t &str) {
  std::cout << str;
  return str;
}

AVMString_t RuntimeLib::ConsoleWriteLn(const AVMString_t &str) {
  std::cout << str << "\n";
  return str;
}

AVMString_t RuntimeLib::ConsoleReadLn() {
  std::string line;
  std::getline(std::cin, line);
  return line;
}
} // namespace avm