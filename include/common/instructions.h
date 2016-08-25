#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

namespace avm {
typedef uint8_t OpCode_t;
enum OpCodes : OpCode_t {
  /**
  nop
    Arguments: none
    RL <=> FL: N/A
    Effect: Ignored
  */
  OpCode_nop = 0,
  /**
  ifl
    Arguments: none
    RL <=> FL: No
    Effect: Increases frame level by one.
  */
  OpCode_ifl,
  /**
  dfl
    Arguments: none
    RL <=> FL: Partly
    Effect: If read level <=> frame level, read level is decremented.
            Frame level will be decremented no matter what.
  */
  OpCode_dfl,
  /**
  irl
    Arguments: none
    RL <=> FL: Yes
    Effects: Read level is incremented.
  */
  OpCode_irl,
  /**
  drl
    Arguments: none
    RL <=> FL: Yes
    Effects: Read level is decremented.
  */
  OpCode_drl,
  /**
  irlt
  Arguments: none
  RL <=> FL: Yes
  Effects: If the top-most value on the stack is true, then read level will
  be incremented. The value will be stored for later use.
  */
  OpCode_irl_if_true,
  /**
  irlf
  Arguments: none
  RL <=> FL: Yes
  Effects: If the last stored "if" value is false, then the read level will be
  incremented.
  */
  OpCode_irl_if_false,
  /**
  addr
    Arguments: ID (u32), Address (u64)
    RL <=> FL: Yes
    Effects: Stores the address for later use, with ID as the key.
  */
  OpCode_store_address,
  /**
  jmpb
    Arguments: ID (u32)
    RL <=> FL: Yes
    Effects: If read level <=> frame level, the stream jumps to the address
             stored by the key ID.
  */
  OpCode_jump,
  /**
  jmpbt
    Arguments: ID (u32)
    RL <=> FL: Yes
    Effects: Same as jmpb, but is only executed if the last condition is
             stored as 1.
  */
  OpCode_jump_if_true,
  /**
  jmpbf
    Arguments: ID (u32)
    RL <=> FL: Yes
    Effects: Same as jmpb, but is only executed if the last condition is
             stored as 0.
  */
  OpCode_jump_if_false,
  /**
  clro
    Not used in this implementation.
  */
  OpCode_clear_object,
  /**
  delo
    Not used in this implementation.
  */
  OpCode_delete_local,
  /**
  try
    Arguments: Catch ID (u32)
    RL <=> FL: Yes
    Effects: Tells the VM that it is now in an exception handling state, and upon error,
             to jump to the code address of the catch id.
  */
  OpCode_try_catch_block,
  /**
  store
    Arguments: Length (i32), Name (String)
    RL <=> FL: Yes
    Effects: Stores the top value from the stack as a local object, denoted
             by the given name. If the top value is a temporary object such as
             a string or number, the value will be stored as a copy. If it is an
             object however, it will be stored as a reference. The top value will
             be automatically popped from the stack.
  */
  OpCode_store_as_local,
  /**
  newv
    Arguments: Length (i32), Name (String)
    RL <=> FL: Yes
    Effects:
      Currently: Creates a new variable and stores it as a local, denoted by
                 the given name.

      In the future: Creates a new variable and pushes it onto the stack. Will
                     likely not take a string as an argument.
  */
  OpCode_new_variable,
  /**
  newn
    Arguments: Length (i32), Type (String)
    RL <=> FL: Yes
    \todo Implement this
  */
  OpCode_new_native_object,
  /**
  idx
    Arguments: None
    RL <=> FL: Yes
    Effects: Pops the two top-most values from the stack. The first value
             is the index, and the second value is the object being accessed.
  */
  OpCode_array_index,
  /**
  newm
    Arguments: Length (i32), Name (String)
    RL <=> FL: Yes
    Effects: Adds a new sub-value to the top item on the stack. It is not
             popped from the stack.
  */
  OpCode_new_member,
  /**
  mbr
    Arguments: Length (i32), Name (String)
    RL <=> FL: Yes
    Effects: Loads a member with the given name from the top value on the stack.
  */
  OpCode_load_member,
  /**
  nes
    Arguments: none
    RL <=> FL: YES
    Effects: Creates an new empty structure type object, and pushes it to the stack.
  */
  OpCode_new_structure,
  /**
  newf
    Arguments: Is Global (u8), No. Arguments (u32), Is Variadic (u8), Address (u64)
    RL <=> FL: Yesl
    Effects: Creates a new function and pushes it onto the stack. If the function is
             global, the stored address will be read from the bytecode. If it is not,
             the address will be stored by the run-time position in the bytecode stream.
  */
  OpCode_new_function,
  /**
  ivk
    Arguments: none
    RL <=> FL: No
    Effects: Attempts to invoke the top value from the stack. It is then
             subsequently popped from the stack.
  */
  OpCode_invoke_object,
  /**
  ret
    Arguments: none
    RL <=> FL: Yes
    Effects: Frame level is decreased, as well as read level. The stream is
             moved to the top-most saved jump position, which is then popped.
  */
  OpCode_return,
  /**
  leave
    Arguments: none
    RL <=> FL: Yes
    Effects: Frame level, as well as read level will be decremented.
             \todo Look into this, decreasing frame level automatically decreases read level.
  */
  OpCode_leave,
  /**
  break
    Arguments: No. Levels (i32)
    RL <=> FL: Yes
    Effects: The stored "if" value is changed to false, and read level is decreased by
             the given number of levels.
  */
  OpCode_break,
  /**
  cont
    Arguments: No. Levels (i32)
    RL <=> FL: Yes
    Effects: The stored "if" value is changed to true, and read level is decreased by
             the given number of levels.
  */
  OpCode_continue,
  /**
  echo
    Arguments: No. Arguments (u32)
    RL <=> FL: Yes
    Effects: Each argument is popped from the stack, and printed to the screen as a
             string.
  */
  OpCode_print,
  /**
  local
    Arguments: Length (i32), Name (String)
    RL <=> FL: Yes
    Effects: The object is loaded onto the stack, from the local variables.
  */
  OpCode_load_local,
  /**
  intn
    Arguments: Value (i64)
    RL <=> FL: Yes
    Effects: The value is loaded onto the stack as a temporary variable.
  */
  OpCode_load_integer,
  /**
  float
    Arguments: Value (double)
    RL <=> FL: Yes
    Effects: The value is loaded onto the stack as a temporary variable.
  */
  OpCode_load_float,
  /**
  str
    Arguments: Length (i32), Name (String)
    RL <=> FL: Yes
    Effects: The value is loaded onto the stack as a temporary variable.
  */
  OpCode_load_string,
  /**
  nil
    Arguments: none
    RL <=> FL: Yes
    Effects: The value is loaded onto the stack as a temporary variable.
  */
  OpCode_load_null,
  /**
  pop
    Arguments: none
    RL <=> FL: Yes
    Effects: The last item will be removed from the stack.
  */
  OpCode_pop,
  /**
  neg
    Arguments: none
    RL <=> FL: Yes
    Effects: The '-' operator is applied to the top-most value of the stack.
  */
  OpCode_unary_minus,
  /**
  not
    Arguments: none
    RL <=> FL: Yes
    Effects: The '!' operator is applied to the top-most value of the stack.
  */
  OpCode_unary_not,
  /**
  pow
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'pow(a,b)' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_pow,
  /**
  add
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a+b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_add,
  /**
  sub
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a-b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_sub,
  /**
  mul
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a*b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_mul,
  /**
  div
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a/b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_div,
  /**
  mod
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a%b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_mod,
  /**
  and
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a&&b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_and,
  /**
  or
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a||b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_or,
  /**
  eql
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a==b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_eql,
  /**
  neql
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a!=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_neql,
  /**
  lt
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a<b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_less,
  /**
  gt
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a>b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_greater,
  /**
  lte
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a<=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_less_eql,
  /**
  gte
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a>=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_greater_eql,
  /**
  band
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a&b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_bit_and,
  /**
  bor
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a|b' operation is computed using the two top-most values
    of the stack. They are both popped from the stack, with the result being
    pushed onto it.
  */
  OpCode_bit_or,
  /**
  xor
  Arguments: none
  RL <=> FL: Yes
  Effects: The 'a^b' operation is computed using the two top-most values
  of the stack. They are both popped from the stack, with the result being
  pushed onto it.
  */
  OpCode_bit_xor,
  /**
  shl
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a<<b' operation is computed using the two top-most values
    of the stack. They are both popped from the stack, with the result being
    pushed onto it.
  */
  OpCode_left_shift,
  /**
  shr
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a>>b' operation is computed using the two top-most values
    of the stack. They are both popped from the stack, with the result being
    pushed onto it.
  */
  OpCode_right_shift,
  /**
  set
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_assign,
  /**
  seta
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a+=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_add_assign,
  /**
  sets
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a-=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_sub_assign,
  /**
  setm
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a*=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_mul_assign,
  /**
  setd
    Arguments: none
    RL <=> FL: Yes
    Effects: The 'a/=b' operation is computed using the two top-most values
             of the stack. They are both popped from the stack, with the result being
             pushed onto it.
  */
  OpCode_div_assign
};
} // namespace avm

#endif