#ifndef AVM_H
#define AVM_H

#include <common/instructions.h>
#include <common/types.h>

#include <detail/vm_state.h>
#include <detail/variable.h>
#include <detail/function.h>
#include <detail/native_function.h>
#include <detail/object.h>
#include <detail/frame.h>
#include <detail/heap.h>

#include <string>
#include <stack>
#include <vector>
#include <map>
#include <utility>
#include <memory>

namespace avm {
typedef Variable &(Variable::*BinOp_t)(VMState *, Variable *);
typedef Variable &(Variable::*UnOp_t)(VMState *);

class VMInstance {
public:
    VMInstance();
    ~VMInstance();

    // Allocates a new stack frame and increases the current level
    void OpenFrame();
    // Deletes the current stack frame and decreases the current level
    void CloseFrame();

    // Creates a new object with integer value and pushes it to the stack
    void PushInt(AVMInteger_t);
    // Creates a new object with float value and pushes it to the stack
    void PushFloat(AVMFloat_t);
    // Creates a new object with string value and pushes it to the stack
    void PushString(const AVMString_t &);
    // Pushes a reference to the stack
    void PushReference(Reference);
    // Pop the top object from the stack
    void PopStack();

    // Performs an operation on the last two objects in the stack.
    // The result will be pushed onto the stack
    void Operation(BinOp_t);
    // Performs an operation on the last object in the stack.
    // The result will be pushed onto the stack
    void Operation(UnOp_t);
    // Performs an assigment on the last two objects in the stack.
    // The result will be pushed onto the stack
    void Assignment();
    // Performs an assignment operation on the last two objects in the stack.
    // The result will be pushed onto the stack
    void Assignment(BinOp_t);
    // Prints the top object from the stack.
    void PrintObjects(size_t);

    // Runs the GC immediately (mark and sweep)
    void GC();
    // Runs the GC if conditions are met
    void SuggestGC();

    // Handle instructions
    void HandleInstruction(Opcode_t opcode);
    // Execute instructions in the stream until the end is reached
    void Execute(ByteStream *);

    VMState *state;

    /** Bind a function with no arguments, and a return type */
    /*template <typename ReturnType>
    void BindFunction(const AVMString_t &name, ReturnType(*ptr) (void)) {
      Reference ref(*state->heap.AllocObject<NativeFunc_NoArgs<ReturnType>>(ptr));
      state->frames[state->frame_level]->locals.push_back({ name, ref });
    }*/

    /** Bind a function with 1 argument, and a return type */
    /*template <typename ReturnType, typename T1>
    void BindFunction(const AVMString_t &name, ReturnType(*ptr) (T1)) {
      Reference ref(*state->heap.AllocObject<NativeFunc_OneArg<ReturnType, T1>>(ptr));
      state->frames[state->frame_level]->locals.push_back({ name, ref });
    }*/

    /** Bind a function with 2 arguments, and a return type */
    /*template <typename ReturnType, typename T1, typename T2>
    void BindFunction(const AVMString_t &name, ReturnType(*ptr) (T1, T2)) {
      Reference ref(*state->heap.AllocObject<NativeFunc_TwoArgs<ReturnType, T1, T2>>(ptr));
      state->frames[state->frame_level]->locals.push_back({ name, ref });
    }*/

    void BindFunction(const AVMString_t &name, void(*ptr) (VMState*, Object**, uint32_t))
    {
        Reference ref(*state->heap.AllocObject<NativeFunc>(ptr));
        state->frames[state->frame_level]->locals.push_back({ name, ref });
    }

private:
    // GC Mark all objects
    void MarkObjects();

    // Create an instance of a natively binded class type
    bool NewNativeObject(const AVMString_t &name);
};
} // namespace avm

#endif