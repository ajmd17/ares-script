#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <memory>

#include <detail/variable.h>

namespace avm {
class VMState;

class Array : public Object {
public:
    Array();

    void invoke(VMState *, uint32_t);

    virtual Reference Clone(VMState *state);

    std::string ToString() const;
    std::string TypeString() const;
};

typedef std::shared_ptr<Array> arraylist_ptr;
} // namespace avm

#endif