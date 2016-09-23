#include <bytecode_generator.h>

#include <iostream>
#include <exception>
#include <memory>
#include <map>
#include <utility>

namespace avm {
BytecodeGenerator::BytecodeGenerator(const InstructionStream &bstream, const std::vector<Label> &labels)
    : bstream(bstream),
      labels(labels)
{
}

bool BytecodeGenerator::Emit(std::ostream &filestream)
{
    // write signature
    filestream.write(ARES_MAGIC, ARES_MAGIC_LEN);
    filestream.write(ARES_VERSION, ARES_VERSION_LEN);

    uint64_t label_offset = (uint64_t)filestream.tellp();
    for (Label &label : labels) {
        label_offset += sizeof(Opcode_t); // cmd type
        label_offset += sizeof(uint32_t); // blockid
        label_offset += sizeof(uint64_t); // block pos
    }

    for (Label &label : labels) {
        // store addresses of each label at top of file
        Opcode_t opcode = Opcode_store_address;
        uint32_t id = (uint32_t)label.id;
        uint64_t addr = (uint64_t)label.location + label_offset;

        filestream.write((char*)&opcode, sizeof(Opcode_t));
        filestream.write((char*)&id, sizeof(uint32_t));
        filestream.write((char*)&addr, sizeof(uint64_t));
    }

    for (Instruction<> &ins : bstream.instructions) {
        switch (ins.data.back()[0]) {
            // FALLTHROUGH
        case Opcode_ifl:
        case Opcode_dfl:
        case Opcode_irl:
        case Opcode_drl:
        case Opcode_irl_if_true:
        case Opcode_irl_if_false:
            ins.Write(filestream);
            break;
        case Opcode_store_address:
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
        case Opcode_jump:
        case Opcode_jump_if_true:
        case Opcode_jump_if_false:
        case Opcode_try_catch_block:
        case Opcode_store_as_local:
        case Opcode_new_native_object:
        case Opcode_array_index:
        case Opcode_new_member:
        case Opcode_load_member:
        case Opcode_new_structure:
        case Opcode_new_function:
        case Opcode_invoke_object:
        case Opcode_return:
        case Opcode_leave:
        case Opcode_break:
        case Opcode_continue:
        case Opcode_print:
        case Opcode_load_local:
        case Opcode_load_field:
        case Opcode_load_integer:
        case Opcode_load_float:
        case Opcode_load_string:
        case Opcode_load_null:
        case Opcode_pop:
        case Opcode_unary_minus:
        case Opcode_unary_not:
        case Opcode_add:
        case Opcode_sub:
        case Opcode_mul:
        case Opcode_div:
        case Opcode_mod:
        case Opcode_pow:
        case Opcode_and:
        case Opcode_or:
        case Opcode_eql:
        case Opcode_neql:
        case Opcode_less:
        case Opcode_greater:
        case Opcode_less_eql:
        case Opcode_greater_eql:
        case Opcode_bit_and:
        case Opcode_bit_or:
        case Opcode_bit_xor:
        case Opcode_left_shift:
        case Opcode_right_shift:
        case Opcode_assign:
        case Opcode_add_assign:
        case Opcode_sub_assign:
        case Opcode_mul_assign:
        case Opcode_div_assign:
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