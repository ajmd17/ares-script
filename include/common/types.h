#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <cstdint>

namespace avm {
typedef int32_t AVMInteger_t;
typedef double AVMFloat_t;
typedef char achar;
typedef std::basic_string<achar> AVMString_t;
typedef uint8_t Opcode_t;
}

#endif