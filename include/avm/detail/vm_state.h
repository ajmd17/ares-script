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

namespace avm {
class VMInstance;
class VMState {
public:
  VMState(VMInstance *vm);
  ~VMState();

  void HandleException(const Exception &except);

  // Default value for maxObjects
  static const int GC_MAX_OBJECTS_DEFAULT;
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
  //std::map<int, Frame*> frames;
  // Are we currently able to handle exceptions, or just crash?
  bool can_handle_exceptions;
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