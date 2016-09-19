#ifndef STACK_VALUE_H
#define STACK_VALUE_H

#include <common/types.h>

namespace avm {
union StackValue {
  AVMInteger_t int_value;
  AVMFloat_t float_value;
};
} // namespace avm

#endif