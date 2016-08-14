#include <detail/heap.h>
#include <iostream>

namespace avm {
Heap::Heap() : head(nullptr), num_objects(0) {
}

Heap::~Heap() {
}

ObjectPtr *Heap::AllocNull() {
  auto *ptr = new HeapObject();
  ptr->id = num_objects++;
  ptr->obj = nullptr;
  if (head != nullptr) {
    head->after = ptr;
  }
  ptr->before = head;
  head = ptr;
  return &ptr->obj;
}

void Heap::Sweep() {
  HeapObject *last = head;
  while (last != nullptr) {
    if (last->obj != nullptr) {
      if (!(last->obj->flags & Object::FLAG_MARKED)) {
        delete last->obj;
        last->obj = nullptr;
        goto deleteholder;
      } else {
        last->obj->flags &= ~Object::FLAG_MARKED;
        last = last->before;
        continue;
      }
    }

  deleteholder:
    HeapObject *after = last->after;
    if (after != nullptr) {
      HeapObject *before = last->before;
      before->after = after;
      after->before = before;
      delete last;
      --num_objects;
      last = before;
    } else if (after == nullptr) {
      HeapObject *before = last->before;
      before->after = nullptr;
      delete last;
      --num_objects;
      head = before; // head is now the object before it
      last = head;
    }
  }
}

void Heap::DumpHeap() const {
  HeapObject *temp_head = head;
  while (temp_head != nullptr) {
    std::cout <<
      "#" << temp_head->id <<
      "\t" << temp_head->obj;

    if (temp_head->obj != nullptr) {
      std::cout <<
        "\t" << temp_head->obj->flags <<
        "\t" << temp_head->obj->ToString();
    }
    std::cout << "\n";

    temp_head = temp_head->before;
  }
}

uint32_t Heap::NumObjects() const {
  return num_objects;
}
} // namespace avm