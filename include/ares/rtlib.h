#ifndef RTLIB_H
#define RTLIB_H

#include <loadlib.h>
#include <common/types.h>
#include <avm/detail/variable.h>
#include <avm/detail/vm_state.h>
#include <avm/detail/check_args.h>

#include <cstdio>

namespace ares {
using namespace avm;
class RuntimeLib {
public:
    static LibLoader *libloader;

    static void FileIO_open(VMState *state, Object **args, uint32_t argc); // takes 2 args
    static void FileIO_write(VMState *state, Object **args, uint32_t argc); // takes 2 args
    static void FileIO_read(VMState *state, Object **args, uint32_t argc); // takes 2 args
    static void FileIO_close(VMState *state, Object **args, uint32_t argc); // takes 1 args

    static void Runtime_loadlib(VMState *state, Object **args, uint32_t argc); // takes 1 args
    static void Runtime_loadfunc(VMState *state, Object **args, uint32_t argc); // takes 2 args
    static void Runtime_invoke(VMState *state, Object **args, uint32_t argc); // takes atleast 1 args

    static void Console_println(VMState *state, Object **args, uint32_t argc); // takes 1 args
    //static void Console_printf(VMState *state, Object **args, uint32_t argc); // takes atleast 1 args
    static void Console_readln(VMState *state, Object **args, uint32_t argc); // takes 0 args
    static void Console_system(VMState *state, Object **args, uint32_t argc); // takes 1 args

    static void Reflection_typeof(VMState *state, Object **args, uint32_t argc); // takes 1 args

    static void Convert_toString(VMState *state, Object **args, uint32_t argc); // takes 1 args
    static void Convert_toInt(VMState *state, Object **args, uint32_t argc); // takes 1 args
    static void Convert_toFloat(VMState *state, Object **args, uint32_t argc); // takes 1 args
    static void Convert_toBool(VMState *state, Object **args, uint32_t argc); // takes 1 args
};
}

#endif