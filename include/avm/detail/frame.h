#ifndef FRAME_H
#define FRAME_H

#include <stack>
#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <iostream>

#include <detail/object.h>
#include <detail/reference.h>
#include <common/types.h>

namespace avm {
static const int AVM_LEVEL_GLOBAL = 0;

class Frame {
public:
  Frame();
  ~Frame();

  bool GetLocal(const AVMString_t &name, Reference &out);

  // Local objects to this frame
  std::vector<std::pair<AVMString_t, Reference>> locals;
  // Last result from a conditional statement
  bool last_cond;
  // Has an exception occured
  bool exception_occured;
};
} // namespace avm

#endif