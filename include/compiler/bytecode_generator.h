#ifndef BYTECODE_GENERATOR_H
#define BYTECODE_GENERATOR_H

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <cstdint>

#include <detail/state.h>
#include <common/bytecodes.h>

namespace avm {
class BytecodeGenerator {
public:
    BytecodeGenerator(const InstructionStream &bstream, const std::vector<Label> &labels);

    bool Emit(std::ostream &stream);

private:
    InstructionStream bstream;
    std::vector<Label> labels;
};
} // namespace avm

#endif