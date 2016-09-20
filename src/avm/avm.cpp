#include "avm.h"

#include <detail/exception.h>
#include <common/util/logger.h>

namespace avm {
VMInstance::VMInstance()
{
    state = new VMState(this);
}

VMInstance::~VMInstance()
{
    GC();
    //std::cout << "\n\nheap dump:\n\n";
    //state->heap.DumpHeap();
    delete state;
}

bool VMInstance::NewNativeObject(const AVMString_t &name)
{
    /** \todo implement this */
    return false;
}

void VMInstance::OpenFrame()
{
    ++state->frame_level;
    state->frames.push_back(new Frame());
}

void VMInstance::CloseFrame()
{
    delete state->frames.back();
    state->frames.pop_back();
    --state->frame_level;
}

void VMInstance::PushInt(AVMInteger_t value)
{
    auto ref = Reference(*state->heap.AllocNull());

    auto var = new Variable();
    var->Assign(value);
    var->flags |= Object::FLAG_CONST;
    var->flags |= Object::FLAG_TEMPORARY;

    ref.Ref() = var;

    PushReference(ref);
}

void VMInstance::PushFloat(AVMFloat_t value)
{
    auto ref = Reference(*state->heap.AllocNull());

    auto var = new Variable();
    var->Assign(value);
    var->flags |= Object::FLAG_CONST;
    var->flags |= Object::FLAG_TEMPORARY;

    ref.Ref() = var;

    PushReference(ref);
}

void VMInstance::PushString(const AVMString_t &value)
{
    auto ref = Reference(*state->heap.AllocNull());

    auto var = new Variable();
    var->Assign(value);
    var->flags |= Object::FLAG_CONST;
    var->flags |= Object::FLAG_TEMPORARY;

    ref.Ref() = var;

    PushReference(ref);
}

void VMInstance::PushReference(Reference ref)
{
    state->stack.push_back(ref);
}

/** Pops the top value from the stack.
    If the object is marked temporary, then it is deleted
*/
void VMInstance::PopStack()
{
    if (state->stack.back().Ref()->flags & Object::FLAG_TEMPORARY) {
        state->stack.back().DeleteObject();
    }
    state->stack.pop_back();
}

void VMInstance::Operation(BinOp_t op)
{
    auto right = state->stack.back(); state->stack.pop_back();
    auto left = state->stack.back(); state->stack.pop_back();

    auto result = left.Ref()->Clone(state);

    Variable *right_var = dynamic_cast<Variable*>(right.Ref());
    if (!right_var) {
        state->HandleException(TypeException(right.Ref()->TypeString()));
    }
    try {
        // todo dynamic_cast to make sure it is Variable type
        ((*static_cast<Variable*>(result.Ref())).*op)(state, right_var);
    } catch (const std::exception &ex) {
        state->HandleException(Exception(ex.what()));
    }

    if (right.Ref()->flags & Object::FLAG_TEMPORARY) {
        right.DeleteObject();
    }
    if (left.Ref()->flags & Object::FLAG_TEMPORARY) {
        left.DeleteObject();
    }

    result.Ref()->flags |= Object::FLAG_TEMPORARY;
    PushReference(Reference(result));
}

void VMInstance::Operation(UnOp_t op)
{
    auto top = state->stack.back(); state->stack.pop_back();

    auto result = top.Ref()->Clone(state);
    try {
        // todo dynamic_cast to amke sure it is Variable type
        ((*static_cast<Variable*>(result.Ref())).*op)(state);
    } catch (const std::exception &ex) {
        state->HandleException(Exception(ex.what()));
    }

    if (top.Ref()->flags & Object::FLAG_TEMPORARY) {
        top.DeleteObject();
    }

    result.Ref()->flags |= Object::FLAG_TEMPORARY;
    PushReference(result);
}

void VMInstance::Assignment()
{
    auto right = state->stack.back(); state->stack.pop_back();
    auto left = state->stack.back();

    bool is_temp = (left.Ref() != nullptr) && (left.Ref()->flags & Object::FLAG_TEMPORARY);

    if (right.Ref() == nullptr) {
        state->HandleException(NullRefException());
    }

    if (left.Ref() != nullptr && (left.Ref()->flags & Object::FLAG_CONST)) {
        state->HandleException(ConstException());
    }

    left.DeleteObject(); // deallocate memory and set to null
    std::swap(left.Ref(), right.Ref()->Clone(state).Ref()); // change left ref to cloned value

    if (is_temp) {
        left.Ref()->flags |= Object::FLAG_TEMPORARY;
    }

    if (right.Ref()->flags & Object::FLAG_TEMPORARY) {
        right.DeleteObject();
    }
}

void VMInstance::Assignment(BinOp_t op)
{
    auto right = state->stack.back(); state->stack.pop_back();
    auto left = state->stack.back();

    if (left.Ref()->flags & Object::FLAG_CONST) {
        state->HandleException(ConstException());
    }

    Variable *left_var = dynamic_cast<Variable*>(left.Ref());
    if (!left_var) {
        state->HandleException(NullRefException());
    }

    Variable *right_var = dynamic_cast<Variable*>(right.Ref());
    if (!right_var) {
        state->HandleException(TypeException(right.Ref()->TypeString()));
    }

    ((*left_var).*op)(state, right_var);

    if (right.Ref()->flags & Object::FLAG_TEMPORARY) {
        right.DeleteObject();
    }
}

void VMInstance::PrintObjects(size_t nargs)
{
    for (size_t i = 0; i < nargs; i++) {
        auto top = state->stack.back(); state->stack.pop_back();

        if (top.Ref() == nullptr) {
            state->HandleException(NullRefException());
        }

        std::string s(top.Ref()->ToString());
        /*std::wstring ws(s.size(), L' ');
        ws.resize(std::mbstowcs(&ws[0], s.c_str(), s.size()));
        output << ws;*/
        std::cout << s;

        if (top.Ref()->flags & Object::FLAG_TEMPORARY) {
            top.DeleteObject();
        }
    }
}

void VMInstance::GC()
{
    DebugLog("run gc");
    MarkObjects();
    state->heap.Sweep();
}

void VMInstance::SuggestGC()
{
    DebugLog("suggest gc");
    if (state->heap.NumObjects() >= state->max_objects) {
        GC();

        if (state->max_objects < GC_THRESHOLD_MAX) {
            state->max_objects += GC_THRESHOLD_STEP;
        }
    }
}

void VMInstance::MarkObjects()
{
    for (auto &&it : state->stack) {
        it.Ref()->Mark();
    }

    // start at current level
    int start = state->frame_level;
    while (start >= AVM_LEVEL_GLOBAL) {
        if (state->frames[start]) {
            for (auto &&it : state->frames[start]->locals) {
                it.second.Ref()->Mark();
            }
        }
        --start;
    }
}

/** In the AVM, code is executed on the condition that the "read level" is
    equal to the "frame level". The frame level is typically incremented where
    the original source code would contain an open curly brace. That way, when an
    "if" statement is encountered, the code inside the body is not executed unless
    the read level is equal to the frame level. Thus, in this case, the read level
    should only be incremented only if the conditions within the "if" statement
    evaluate to true.
*/
void VMInstance::HandleInstruction(OpCode_t opcode)
{
    switch (opcode) {
    case OpCode_ifl:
    {
        OpenFrame();
        DebugLog("Increase frame level to: %d. Read level is: %d", state->frame_level, state->read_level);

        break;
    }
    case OpCode_dfl:
    {
        bool should_gc = false;
        if (state->read_level == state->frame_level) {
            should_gc = true;
            --state->read_level;
            DebugLog("Decrease read level to: %d", state->read_level);
        }

        CloseFrame();
        DebugLog("Decrease frame level to: %d", state->frame_level);

        if (should_gc) {
            // collect garbage to free variables from previous frame
            SuggestGC();
        }

        break;
    }
    case OpCode_irl:
    {
        if (state->read_level == state->frame_level) {
            ++state->read_level;
            DebugLog("Increase read level to: %d", state->read_level);
        }

        break;
    }
    case OpCode_drl:
    {
        if (state->read_level == state->frame_level) {
            uint8_t count;
            state->stream->Read(&count);

            state->read_level -= count;
            DebugLog("Decrease read level to: %d", state->read_level);
        } else {
            state->stream->Skip(sizeof(uint8_t));
        }

        break;
    }
    case OpCode_irl_if_true:
    {
        if (state->read_level == state->frame_level) {
            auto *frame = state->frames[state->frame_level];

            Variable *top = dynamic_cast<Variable*>(state->stack.back().Ref());
            if (!top) {
                state->HandleException(NullRefException());
            }

            bool result = false;
            try {
                result = (top ? top->Cast<bool>() : false);
            } catch (const std::exception &ex) {
                state->HandleException(Exception(ex.what()));
            }

            DebugLog("If result: %s", (result ? "true" : "false"));
            PopStack();

            frame->last_cond = result;

            if (result) {
                ++state->read_level;
                DebugLog("Increase read level to: %d", state->read_level);
            }
        }

        break;
    }
    case OpCode_irl_if_false:
    {
        if (state->read_level == state->frame_level) {
            if (!state->frames[state->frame_level]->last_cond) {
                ++state->read_level;
                DebugLog("Increase read level to: %d", state->read_level);
            }
        }

        break;
    }
    case OpCode_try_catch_block:
    {
        if (state->read_level == state->frame_level) {
            auto old_frame_level = state->frame_level;
            auto old_read_level = state->read_level;

            bool exception_occured = false;

            ++state->read_level;
            state->can_handle_exceptions = true;

            do {
                OpCode_t next_ins;
                state->stream->Read(&next_ins);
                state->vm->HandleInstruction(next_ins);

                if (state->frames[state->frame_level]->exception_occured) {
                    exception_occured = true;
                    // exception will now be handled, so reset the flag
                    state->frames[state->frame_level]->exception_occured = false;
                    state->read_level = old_read_level;
                }
            } while (state->frame_level != old_frame_level);

            state->can_handle_exceptions = false;

            // next segment is the catch block, so if an error occured, we now handle it
            if (exception_occured) {
                ++state->read_level;
                do {
                    OpCode_t next_ins;
                    state->stream->Read(&next_ins);
                    state->vm->HandleInstruction(next_ins);
                } while (state->frame_level != old_frame_level);
            }
        }

        break;
    }
    case OpCode_store_address:
    {
        uint32_t id;
        state->stream->Read(&id);

        uint64_t address;
        state->stream->Read(&address);
        state->block_positions[id] = address;

        DebugLog("Create block: %d at position: %d", id, address);

        break;
    }
    case OpCode_jump:
    {
        if (state->read_level == state->frame_level) {
            uint32_t id;
            state->stream->Read(&id);

            auto position = state->block_positions[id];
            DebugLog("Go to block: %u at position: %d", id, position);

            state->stream->Seek(position);
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case OpCode_jump_if_true:
    {
        if (state->read_level == state->frame_level &&
            state->frames[state->frame_level]->last_cond) {
            uint32_t id;
            state->stream->Read(&id);

            auto position = state->block_positions[id];
            DebugLog("Go to block: %u at position: %d", id, position);

            state->stream->Seek(position);
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case OpCode_jump_if_false:
    {
        if (state->read_level == state->frame_level &&
            !state->frames[state->frame_level]->last_cond) {
            uint32_t id;
            state->stream->Read(&id);

            auto position = state->block_positions[id];
            DebugLog("Go to block: %u at position: %d", id, position);

            state->stream->Seek(position);
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case OpCode_store_as_local:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DebugLog("Storing top in local: %s", str);

            auto frame = state->frames[state->frame_level];
            auto top = state->stack.back(); state->stack.pop_back();

            Reference ref;
            if (top.Ref()->flags & Object::FLAG_TEMPORARY) {
                ref = top.Ref()->Clone(state); // temp values like ints or strings are copied
                // after cloning the object, delete the old one
                top.DeleteObject();
            } else {
                ref = top; // objects are copied as a reference
                // inc ref count?
            }

            frame->locals.push_back({ str, ref });

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case OpCode_new_native_object:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DebugLog("Create native class instance: %s", str);
            NewNativeObject(str);

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case OpCode_array_index:
    {
        auto right = state->stack.back(); state->stack.pop_back();
        auto left = state->stack.back(); state->stack.pop_back();

        Variable *right_var = dynamic_cast<Variable*>(right.Ref());
        if (!right_var) {
            state->HandleException(TypeException(right.Ref()->TypeString()));
        }

        try {
            Reference ref;
            if (right_var->type == Variable::Type_int) {
                left.Ref()->GetFieldReference(state, right_var->Cast<AVMInteger_t>(), ref);
            } else if (right_var->type == Variable::Type_string) {
                left.Ref()->GetFieldReference(state, right_var->Cast<AVMString_t>(), ref);
            } else {
                throw "invalid index";
            }
            PushReference(ref);
        } catch (const std::exception &ex) {
            state->HandleException(Exception(ex.what()));
        }

        if (right.Ref()->flags & Object::FLAG_TEMPORARY) {
            right.DeleteObject();
        }

        if (left.Ref()->flags & Object::FLAG_TEMPORARY) {
            left.DeleteObject();
        }

        break;
    }
    case OpCode_new_member:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DebugLog("Add member: %s", str);

            auto object = state->stack.back();
            auto ref = Reference(*state->heap.AllocObject<Variable>());
            if (object.Ref()->AddFieldReference(state, str, ref)) {
                PushReference(ref);
            }

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case OpCode_load_member:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DebugLog("Load member: %s", str);

            auto ref = state->stack.back(); state->stack.pop_back();

            Reference member;
            if (ref.Ref()->GetFieldReference(state, str, member)) {
                PushReference(member);
            }

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case OpCode_new_structure:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("New structure");

            auto ref = Reference(*state->heap.AllocNull());
            auto var = new Variable(); /// \todo: Make a unique structure class
            var->type = Variable::Type_struct;
            var->flags |= Object::FLAG_CONST;
            var->flags |= Object::FLAG_TEMPORARY;

            ref.Ref() = var;

            PushReference(ref);
        }
        break;
    }
    case OpCode_new_function:
    {
        if (state->read_level == state->frame_level) {
            uint8_t global;
            state->stream->Read(&global);

            uint32_t nargs;
            state->stream->Read(&nargs);

            uint8_t variadic;
            state->stream->Read(&variadic);

            uint64_t address;
            state->stream->Read(&address);

            DebugLog("Pushing function to stack");

            uint64_t pos = global ? address : static_cast<uint64_t>(state->stream->Position());

            auto ref = Reference(*state->heap.AllocObject<Func>(pos, nargs, static_cast<bool>(variadic)));
            ref.Ref()->flags |= Object::FLAG_TEMPORARY;
            PushReference(ref);
        } else {
            state->stream->Skip(sizeof(uint8_t));
            state->stream->Skip(sizeof(uint32_t));
            state->stream->Skip(sizeof(uint8_t));
            state->stream->Skip(sizeof(uint64_t));
        }

        break;
    }
    case OpCode_invoke_object:
        if (state->read_level == state->frame_level) {
            uint32_t nargs;
            state->stream->Read(&nargs);

            DebugLog("Invoking");

            Reference reference = state->stack.back(); state->stack.pop_back();
            reference.Ref()->invoke(state, nargs);
            if (reference.Ref()->flags & Object::FLAG_TEMPORARY) {
                reference.DeleteObject();
            }
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    case OpCode_return:
    {
        if (state->read_level == state->frame_level) {
            // handled in function class
        }

        break;
    }
    case OpCode_leave:
        if (state->read_level == state->frame_level) {
            DebugLog("Leave block");

            CloseFrame();
            DebugLog("Decrease frame level to: %d", state->frame_level);

            --state->read_level;
            DebugLog("Decrease read level to: %d", state->read_level);
        }

        break;
    case OpCode_break:
    {
        if (state->read_level == state->frame_level) {
            int32_t levels_to_skip;
            state->stream->Read(&levels_to_skip);

            DebugLog("Loop break");
            state->frames[state->frame_level - levels_to_skip]->last_cond = false;
            state->read_level -= levels_to_skip;
        } else {
            state->stream->Skip(sizeof(int32_t));
        }

        break;
    }
    case OpCode_continue:
    {
        if (state->read_level == state->frame_level) {
            int32_t levels_to_skip;
            state->stream->Read(&levels_to_skip);

            DebugLog("Loop continue");
            state->frames[state->frame_level - levels_to_skip]->last_cond = true;
            state->read_level -= levels_to_skip;
        } else {
            state->stream->Skip(sizeof(int32_t));
        }

        break;
    }
    case OpCode_print:
    {
        if (state->read_level == state->frame_level) {
            uint32_t nargs;
            state->stream->Read(&nargs);
            PrintObjects(nargs);
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case OpCode_load_local:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DebugLog("Loading variable: '%s'", str);

            int start = state->frame_level;
            bool found = false;

            while (start >= AVM_LEVEL_GLOBAL) {
                Frame *frame = state->frames[start];

                // Use pointer to pointer so that we have can change type
                Reference ref;
                if (frame->GetLocal(str, ref)) {
                    PushReference(ref);
                    found = true;
                    break;
                }

                --start;
            }

            if (!found) {
                throw std::runtime_error("could not find object");
            }

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case OpCode_load_integer:
    {
        if (state->read_level == state->frame_level) {
            AVMInteger_t value;
            state->stream->Read(&value);

            DebugLog("Load integer: %d", value);
            PushInt(value);
        } else {
            state->stream->Skip(sizeof(AVMInteger_t));
        }

        break;
    }
    case OpCode_load_float:
    {
        if (state->read_level == state->frame_level) {
            AVMFloat_t value;
            state->stream->Read(&value);

            DebugLog("Load float: %f", value);
            PushFloat(value);
        } else {
            state->stream->Skip(sizeof(AVMFloat_t));
        }

        break;
    }
    case OpCode_load_string:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DebugLog("Load string: %s", str);
            PushString(AVMString_t(str));

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case OpCode_load_null:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Load null");

            auto ref = Reference(*state->heap.AllocObject<Variable>());
            ref.Ref()->flags |= Object::FLAG_TEMPORARY;
            PushReference(ref);
        }

        break;
    }
    case OpCode_pop:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Pop stack");
            PopStack();
        }

        break;
    }
    case OpCode_unary_minus:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Unary -");
            Operation(&Variable::Negate);
        }

        break;
    }
    case OpCode_unary_not:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Unary !");
            Operation(&Variable::LogicalNot);
        }

        break;
    }
    case OpCode_add:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary +");
            Operation(&Variable::Add);
        }

        break;
    }
    case OpCode_sub:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary -");
            Operation(&Variable::Subtract);
        }

        break;
    }
    case OpCode_mul:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary *");
            Operation(&Variable::Multiply);
        }

        break;
    }
    case OpCode_div:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary /");
            Operation(&Variable::Divide);
        }

        break;
    }
    case OpCode_mod:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary %");
            Operation(&Variable::Modulus);
        }

        break;
    }
    case OpCode_pow:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary **");
            Operation(&Variable::Power);
        }

        break;
    }
    case OpCode_and:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary &&");
            Operation(&Variable::LogicalAnd);
        }

        break;
    }
    case OpCode_or:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary ||");
            Operation(&Variable::LogicalOr);
        }

        break;
    }
    case OpCode_eql:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary ==");
            Operation(&Variable::Equals);
        }

        break;
    }
    case OpCode_neql:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary !=");
            Operation(&Variable::NotEqual);
        }

        break;
    }
    case OpCode_less:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary <");
            Operation(&Variable::Less);
        }

        break;
    }
    case OpCode_greater:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary >");
            Operation(&Variable::Greater);
        }

        break;
    }
    case OpCode_less_eql:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary <=");
            Operation(&Variable::LessOrEqual);
        }

        break;
    }
    case OpCode_greater_eql:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary >=");
            Operation(&Variable::GreaterOrEqual);
        }

        break;
    }
    case OpCode_bit_and:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary &");
            Operation(&Variable::BitwiseAnd);
        }

        break;
    }
    case OpCode_bit_or:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary |");
            Operation(&Variable::BitwiseOr);
        }

        break;
    }
    case OpCode_bit_xor:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary &");
            Operation(&Variable::BitwiseXor);
        }

        break;
    }
    case OpCode_left_shift:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary <<");
            Operation(&Variable::LeftShift);
        }

        break;
    }
    case OpCode_right_shift:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary >>");
            Operation(&Variable::RightShift);
        }

        break;
    }
    case OpCode_assign:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary =");
            Assignment();
        }

        break;
    }
    case OpCode_add_assign:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary +=");
            Assignment(&Variable::Add);
        }

        break;
    }
    case OpCode_sub_assign:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary -=");
            Assignment(&Variable::Subtract);
        }

        break;
    }
    case OpCode_mul_assign:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary *=");
            Assignment(&Variable::Multiply);
        }

        break;
    }
    case OpCode_div_assign:
    {
        if (state->read_level == state->frame_level) {
            DebugLog("Binary /=");
            Assignment(&Variable::Divide);
        }

        break;
    }
    default:
    {
        auto last_pos = (((unsigned long)state->stream->Position()) - sizeof(OpCode_t));
        std::cout << "Unrecognized instruction '" << (int)opcode << "' at position: " << last_pos << "\n";
        break;
    }
    }
}

void VMInstance::Execute(ByteStream *bs)
{
    state->stream = bs;

    while (state->stream->Position() < state->stream->Max()) {
        OpCode_t ins;
        state->stream->Read(&ins);
        HandleInstruction(ins);
    }
}
} // namespace avm