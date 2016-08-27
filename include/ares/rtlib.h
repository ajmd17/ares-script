#ifndef RTLIB_H
#define RTLIB_H

#include <common/types.h>
#include <avm/detail/variable.h>
#include <avm/detail/vm_state.h>

#include <cstdio>

namespace avm {
class RuntimeLib {
public:
  /*static FILE *OpenFile(const AVMString_t &filepath, const AVMString_t &mode);
  static AVMInteger_t WriteStringToFile(FILE *stream, const AVMString_t &str);
  static AVMString_t ReadStringFromFile(FILE *stream, AVMInteger_t len);
  static AVMInteger_t CloseFile(FILE *stream);

  static AVMInteger_t ConvertToInt(AVMFloat_t f);
  static bool ConvertToBool(AVMInteger_t i);

  static AVMInteger_t SystemRun(const AVMString_t &str);

  static AVMString_t ConsoleWrite(const AVMString_t &str);
  static AVMString_t ConsoleWriteLn(const AVMString_t &str);
  static AVMString_t ConsoleReadLn();*/

  static void FileIO_open(VMState *state, Object *arg1, Object *arg2);
  static void FileIO_write(VMState *state, Object *arg1, Object *arg2);
  static void FileIO_read(VMState *state, Object *arg1, Object *arg2);
  static void FileIO_close(VMState *state, Object *arg1);

  static void Console_println(VMState *state, Object *arg1);
  static void Console_readln(VMState *state);

  static void Reflection_typeof(VMState *state, Object *arg1);

  static void Convert_toString(VMState *state, Object *arg1);
  static void Convert_toInt(VMState *state, Object *arg1);
  static void Convert_toBool(VMState *state, Object *arg1);

};
}

#endif