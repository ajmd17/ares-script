#include <abytecodegen.h>

#include <iostream>
#include <exception>
#include <memory>
#include <map>
#include <utility>

namespace avm {
BytecodeGenerator::BytecodeGenerator(const InstructionStream &bstream) : bstream(bstream) {
}

bool BytecodeGenerator::Emit(std::ostream &filestream) {
  for (auto &&ins : bstream.instructions) {
    switch (ins.data.back()[0]) {
    // FALLTHROUGH
    case ins_ifl:
    case ins_dfl:
    case ins_irl:
    case ins_drl:
    case ins_irl_if_true:
    case ins_irl_if_false:
      ins.Write(filestream);
      break;
    case ins_store_address:
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
    case ins_jump:
    case ins_jump_if_true:
    case ins_jump_if_false:
    case ins_try_catch_block:
    case ins_store_as_local:
    case ins_new_native_object:
    case ins_array_index:
    case ins_new_member:
    case ins_load_member:
      ins.Write(filestream);
      break;
    case ins_new_function:
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
    case ins_invoke_object:
    case ins_invoke_native:
    case ins_return:
    case ins_leave:
    case ins_break:
    case ins_continue:
    case ins_print:
    case ins_load_local:
    case ins_load_integer:
    case ins_load_float:
    case ins_load_string:
    case ins_load_null:
    case ins_pop:
    case ins_unary_minus:
    case ins_unary_not:
    case ins_add:
    case ins_sub:
    case ins_mul:
    case ins_div:
    case ins_mod:
    case ins_pow:
    case ins_and:
    case ins_or:
    case ins_eql:
    case ins_neql:
    case ins_less:
    case ins_greater:
    case ins_less_eql:
    case ins_greater_eql:
    case ins_bit_and:
    case ins_bit_or:
    case ins_bit_xor:
    case ins_left_shift:
    case ins_right_shift:
    case ins_assign:
    case ins_add_assign:
    case ins_sub_assign:
    case ins_mul_assign:
    case ins_div_assign:
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