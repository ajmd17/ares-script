#ifndef VM_STATE_H
#define VM_STATE_H

#include <string>
#include <stack>
#include <vector>
#include <map>
#include <utility>
#include <memory>

#include <detail/frame.h>
#include <detail/heap.h>
#include <detail/byte_stream.h>
#include <detail/object.h>
#include <detail/exception.h>
#include <detail/reference.h>
#include <detail/variable.h>

#define GC_THRESHOLD_MIN 500 // start # of objects before GC.
#define GC_THRESHOLD_MAX 2000 // max out at this number, do not increase threshold
#define GC_THRESHOLD_STEP 100 // how much to increase each time GC runs.

namespace avm {
class VMInstance;
class VMState {
public:
  VMState(VMInstance *vm);
  ~VMState();

  void HandleException(const Exception &except);

  // The current frame level
  int frame_level;
  // The current read level
  int read_level;
  // Saved positions from jumping
  std::stack<unsigned long> jump_positions;
  // Saved block locations, mapped by block ID
  std::map<int, unsigned long> block_positions;
  // The stream that instructions are being read from
  ByteStream *stream;
  // Current number of objects
  size_t num_objects;
  // Maximum number of objects before resize
  size_t max_objects;
  // Frame pointers
  std::vector<Frame*> frames;
  // Are we currently able to handle exceptions, or just crash?
  bool can_handle_exceptions;
  // the stack pointer (temporary value index)
  //size_t sp = 0;
  // Holds temporary values
 // Variable temporaries[200];
  // Holds the heap memory
  Heap heap;
  // Maximum heap memory before the GC is called
  size_t max_heap_size;
  // Pointer to the VM
  VMInstance *vm;

  std::vector<Reference> stack;
};
} // namespace avm

#endif