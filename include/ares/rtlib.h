#ifndef RTLIB_H
#define RTLIB_H

#include <common/types.h>

#include <cstdio>

namespace avm {
class RuntimeLib {
public:
  static FILE *OpenFile(const AVMString_t &filepath, const AVMString_t &mode);
  static AVMInteger_t WriteStringToFile(FILE *stream, const AVMString_t &str);
  static AVMString_t ReadStringFromFile(FILE *stream, AVMInteger_t len);
  static AVMInteger_t CloseFile(FILE *stream);

  static AVMInteger_t ConvertToInt(AVMFloat_t f);
  static bool ConvertToBool(AVMInteger_t i);

  static AVMInteger_t SystemRun(const AVMString_t &str);

  static AVMString_t ConsoleWrite(const AVMString_t &str);
  static AVMString_t ConsoleWriteLn(const AVMString_t &str);
  static AVMString_t ConsoleReadLn();
};
}

#endif