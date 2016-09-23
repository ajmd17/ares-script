#ifndef HEAP_H
#define HEAP_H

#include <detail/object.h>

#include <ostream>

namespace avm {
class Heap {
public:
    Heap();
    ~Heap();

    // allocate a null object
    ObjectPtr *AllocNull();

    template <typename T, typename...Args>
    ObjectPtr *AllocObject(Args &&... args)
    {
        static_assert(std::is_base_of<Object, T>::value, "T must be a derived class of object");

        auto *ptr = new HeapObject();
        ptr->id = num_objects++;
        ptr->obj = new T(args...);
        if (head != nullptr) {
            head->after = ptr;
        }
        ptr->before = head;
        head = ptr;
        return &ptr->obj;
    }

    void Sweep();
    void DumpHeap(std::ostream &os) const;
    uint32_t NumObjects() const;

private:
    struct HeapObject {
        unsigned long id;
        ObjectPtr obj = nullptr;
        HeapObject *before = nullptr;
        HeapObject *after = nullptr;
    };

    HeapObject *head;
    uint32_t num_objects;
};
}

#endif