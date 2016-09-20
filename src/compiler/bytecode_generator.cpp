#include <bytecode_generator.h>

#include <iostream>
#include <exception>
#include <memory>
#include <map>
#include <utility>

namespace avm {
BytecodeGenerator::BytecodeGenerator(const InstructionStream &bstream) : bstream(bstream)
{
}

bool BytecodeGenerator::Emit(std::ostream &filestream)
{
    // write signature
    filestream.write(ARES_MAGIC, strlen(ARES_MAGIC));
    filestream.write(ARES_VERSION, ARES_VERSION_LEN);

    for (auto &&ins : bstream.instructions) {
        switch (ins.data.back()[0]) {
            // FALLTHROUGH
        case OpCode_ifl:
        case OpCode_dfl:
        case OpCode_irl:
        case OpCode_drl:
        case OpCode_irl_if_true:
        case OpCode_irl_if_false:
            ins.Write(filestream);
            break;
        case OpCode_store_address:
        {
            uint64_t addr = 0;
            addr += ins.data.back().size(); // cmd type
            addr += sizeof(uint32_t); // blockid
            addr += sizeof(uint64_t); // block pos

            uint64_t pos = (uint64_t)filestream.tellp();
            uint64_t offset_pos = pos + addr;

            ins.Write(filestream);
            filestream.write((char*)&offset_pos, sizeof(uint64_t)); // block position

            break;
        }
        // FALLTHROUGH
        case OpCode_jump:
        case OpCode_jump_if_true:
        case OpCode_jump_if_false:
        case OpCode_try_catch_block:
        case OpCode_store_as_local:
        case OpCode_new_native_object:
        case OpCode_array_index:
        case OpCode_new_member:
        case OpCode_load_member:
        case OpCode_new_structure:
            ins.Write(filestream);
            break;
        case OpCode_new_function:
        {
            uint64_t addr = 0;
            addr += ins.data.back().size(); // cmd type
            addr += sizeof(uint8_t); // is global
            addr += sizeof(uint32_t); // no. of args
            addr += sizeof(uint8_t); // is variadic
            addr += sizeof(uint64_t); // block pos

            uint64_t pos = (uint64_t)filestream.tellp();
            uint64_t offset_pos = pos + addr;

            ins.Write(filestream);

            filestream.write((char*)&offset_pos, sizeof(uint64_t)); // offset pos

            break;
        }
        // FALLTHROUGH
        case OpCode_invoke_object:
        case OpCode_return:
        case OpCode_leave:
        case OpCode_break:
        case OpCode_continue:
        case OpCode_print:
        case OpCode_load_local:
        case OpCode_load_integer:
        case OpCode_load_float:
        case OpCode_load_string:
        case OpCode_load_null:
        case OpCode_pop:
        case OpCode_unary_minus:
        case OpCode_unary_not:
        case OpCode_add:
        case OpCode_sub:
        case OpCode_mul:
        case OpCode_div:
        case OpCode_mod:
        case OpCode_pow:
        case OpCode_and:
        case OpCode_or:
        case OpCode_eql:
        case OpCode_neql:
        case OpCode_less:
        case OpCode_greater:
        case OpCode_less_eql:
        case OpCode_greater_eql:
        case OpCode_bit_and:
        case OpCode_bit_or:
        case OpCode_bit_xor:
        case OpCode_left_shift:
        case OpCode_right_shift:
        case OpCode_assign:
        case OpCode_add_assign:
        case OpCode_sub_assign:
        case OpCode_mul_assign:
        case OpCode_div_assign:
            ins.Write(filestream);
            break;
        default:
            std::cout << "Unrecognized code: " << (int)ins.data.back()[0] << "\n";
            return false;
        }
    }
    return true;
}
} // namespace avm