#ifndef BYTECODE_GENERATOR_H
#define BYTECODE_GENERATOR_H

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <cstdint>

#include <common/bytecodes.h>

namespace avm {
class BytecodeGenerator {
public:
    BytecodeGenerator(const InstructionStream &bstream);

    bool Emit(std::ostream &stream);

private:
    InstructionStream bstream;
};
} // namespace avm

#endif