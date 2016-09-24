#include "avm.h"

#include <detail/exception.h>
#include <common/util/logger.h>

#include <sstream>

namespace avm {
VMInstance::VMInstance()
{
    state = new VMState(this);
}

VMInstance::~VMInstance()
{
    GC();


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
    DEBUG_LOG("run gc");
    MarkObjects();
    state->heap.Sweep();
}

void VMInstance::SuggestGC()
{
    DEBUG_LOG("suggest gc");
    if (state->heap.NumObjects() >= state->max_objects) {
        GC();

        if (state->max_objects < GC_THRESHOLD_MAX) {
            state->max_objects += GC_THRESHOLD_STEP;
        }
    }
}

void VMInstance::MarkObjects()
{
    for (Reference &ref : state->stack) {
        ref.Ref()->Mark();
    }

    // start at current level
    int start = state->frame_level;
    while (start >= AVM_LEVEL_GLOBAL) {
        if (state->frames[start] != nullptr) {
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
void VMInstance::HandleInstruction(Opcode_t opcode)
{
    switch (opcode) {
    case Opcode_ifl:
    {
        OpenFrame();
        DEBUG_LOG("Increase frame level to: %d. Read level is: %d", state->frame_level, state->read_level);
        break;
    }
    case Opcode_dfl:
    {
        bool should_suggest_gc = false;
        if (state->read_level == state->frame_level) {
            should_suggest_gc = true;
            --state->read_level;
            DEBUG_LOG("Decrease read level to: %d", state->read_level);
        }

        CloseFrame();
        DEBUG_LOG("Decrease frame level to: %d", state->frame_level);

        if (should_suggest_gc) {
            // collect garbage to free variables from previous frame
            SuggestGC();
        }

        break;
    }
    case Opcode_irl:
    {
        if (state->read_level == state->frame_level) {
            ++state->read_level;
            DEBUG_LOG("Increase read level to: %d", state->read_level);
        }

        break;
    }
    case Opcode_drl:
    {
        if (state->read_level == state->frame_level) {
            uint8_t count;
            state->stream->Read(&count);

            state->read_level -= count;
            DEBUG_LOG("Decrease read level to: %d", state->read_level);
        } else {
            state->stream->Skip(sizeof(uint8_t));
        }

        break;
    }
    case Opcode_irl_if_true:
    {
        if (state->read_level == state->frame_level) {
            Frame *frame = state->frames[state->frame_level];

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

            DEBUG_LOG("If result: %s", (result ? "true" : "false"));

            frame->last_cond = result;

            if (result) {
                ++state->read_level;
                DEBUG_LOG("Increase read level to: %d", state->read_level);
            }
        }

        break;
    }
    case Opcode_irl_if_false:
    {
        if (state->read_level == state->frame_level) {
            Frame *frame = state->frames[state->frame_level];

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

            DEBUG_LOG("If result: %s", (result ? "true" : "false"));

            frame->last_cond = result;

            if (!result) {
                ++state->read_level;
                DEBUG_LOG("Increase read level to: %d", state->read_level);
            }
        }

        break;
    }
    case Opcode_try_catch_block:
    {
        if (state->read_level == state->frame_level) {
            int old_frame_level = state->frame_level;
            int old_read_level = state->read_level;

            bool exception_occured = false;

            ++state->read_level;
            state->can_handle_exceptions = true;

            do {
                Opcode_t next_ins;
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
                    Opcode_t next_ins;
                    state->stream->Read(&next_ins);
                    state->vm->HandleInstruction(next_ins);
                } while (state->frame_level != old_frame_level);
            }
        }

        break;
    }
    case Opcode_store_address:
    {
        struct {
            uint32_t id;
            uint64_t address;
        } data;

        state->stream->Read(&data.id);
        state->stream->Read(&data.address);

        state->block_positions[data.id] = data.address;

        DEBUG_LOG("Create block: %d at position: %d", data.id, data.address);

        break;
    }
    case Opcode_jump:
    {
        if (state->read_level == state->frame_level) {
            uint32_t id;
            state->stream->Read(&id);

            auto position = state->block_positions[id];
            DEBUG_LOG("Go to block: %u at position: %d", id, position);

            state->stream->Seek(position);
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case Opcode_jump_if_true:
    {
        /*if (state->read_level == state->frame_level &&
            state->frames[state->frame_level]->last_cond) {
            uint32_t id;
            state->stream->Read(&id);

            auto position = state->block_positions[id];
            DEBUG_LOG("Go to block: %u at position: %d", id, position);

            state->stream->Seek(position);
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }*/

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

            DEBUG_LOG("If result: %s", (result ? "true" : "false"));

            frame->last_cond = result;

            if (result) {
                uint32_t id;
                state->stream->Read(&id);

                auto position = state->block_positions[id];
                DEBUG_LOG("Go to block: %u at position: %d", id, position);
                state->stream->Seek(position);
            } else {
                state->stream->Skip(sizeof(uint32_t));
            }
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case Opcode_jump_if_false:
    {
        /*if (state->read_level == state->frame_level &&
            !state->frames[state->frame_level]->last_cond) {
            uint32_t id;
            state->stream->Read(&id);

            auto position = state->block_positions[id];
            DEBUG_LOG("Go to block: %u at position: %d", id, position);

            state->stream->Seek(position);
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }*/

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

            DEBUG_LOG("If result: %s", (result ? "true" : "false"));

            frame->last_cond = result;

            if (!result) {
                uint32_t id;
                state->stream->Read(&id);

                auto position = state->block_positions[id];
                DEBUG_LOG("Go to block: %u at position: %d", id, position);
                state->stream->Seek(position);
            } else {
                state->stream->Skip(sizeof(uint32_t));
            }
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case Opcode_store_as_local:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DEBUG_LOG("Storing top in local: %s", str);

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
    case Opcode_new_native_object:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DEBUG_LOG("Create native class instance: %s", str);
            NewNativeObject(str);

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case Opcode_array_index:
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
    case Opcode_new_member:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DEBUG_LOG("Add member: %s", str);

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
    case Opcode_load_member:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DEBUG_LOG("Load member: %s", str);

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
    case Opcode_new_structure:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("New structure");

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
    case Opcode_new_function:
    {
        if (state->read_level == state->frame_level) {
            struct {
                uint32_t num_args;
                uint8_t is_variadic;
                uint32_t id;
            } function_info;

            state->stream->Read(&function_info.num_args);
            state->stream->Read(&function_info.is_variadic);
            state->stream->Read(&function_info.id);

            DEBUG_LOG("Pushing function to stack");

            uint64_t pos = state->block_positions[function_info.id];

            auto ref = Reference(*state->heap.AllocObject<Func>(pos, 
                function_info.num_args, (bool)function_info.is_variadic));

            ref.Ref()->flags |= Object::FLAG_TEMPORARY;
            PushReference(ref);
        } else {
            state->stream->Skip(sizeof(uint8_t));
            state->stream->Skip(sizeof(uint32_t));
            state->stream->Skip(sizeof(uint8_t));
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    }
    case Opcode_invoke_object:
        if (state->read_level == state->frame_level) {
            uint32_t nargs;
            state->stream->Read(&nargs);

            DEBUG_LOG("Invoking");

            Reference reference = state->stack.back(); state->stack.pop_back();
            reference.Ref()->invoke(state, nargs);
            if (reference.Ref()->flags & Object::FLAG_TEMPORARY) {
                reference.DeleteObject();
            }
        } else {
            state->stream->Skip(sizeof(uint32_t));
        }

        break;
    case Opcode_return:
    {
        // handled in function class
        break;
    }
    case Opcode_leave:
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Leave block");

            CloseFrame();
            DEBUG_LOG("Decrease frame level to: %d", state->frame_level);

            --state->read_level;
            DEBUG_LOG("Decrease read level to: %d", state->read_level);
        }

        break;
    case Opcode_break:
    {
        if (state->read_level == state->frame_level) {
            int32_t levels_to_skip;
            state->stream->Read(&levels_to_skip);

            DEBUG_LOG("Loop break");
            state->frames[state->frame_level - levels_to_skip]->last_cond = false;
            state->read_level -= levels_to_skip;
        } else {
            state->stream->Skip(sizeof(int32_t));
        }

        break;
    }
    case Opcode_continue:
    {
        if (state->read_level == state->frame_level) {
            int32_t levels_to_skip;
            state->stream->Read(&levels_to_skip);

            DEBUG_LOG("Loop continue");
            state->frames[state->frame_level - levels_to_skip]->last_cond = true;
            state->read_level -= levels_to_skip;
        } else {
            state->stream->Skip(sizeof(int32_t));
        }

        break;
    }
    case Opcode_print:
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
    case Opcode_load_local:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DEBUG_LOG("Loading variable: '%s'", str);

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
    case Opcode_load_field:
    {
        if (state->read_level == state->frame_level) {
            struct {
                int32_t frame_index_difference;
                int32_t field_index;
            } field_info;

            state->stream->Read(&field_info.frame_index_difference);
            state->stream->Read(&field_info.field_index);

            int32_t frame_index = state->frame_level - field_info.frame_index_difference;

            DEBUG_LOG("Loading field #%d from frame #%d", field_info.field_index, frame_index);

            Frame *frame = state->frames[frame_index];
            PushReference(frame->locals[field_info.field_index].second);

        } else {
            state->stream->Skip(sizeof(int32_t));
            state->stream->Skip(sizeof(int32_t));
        }

        break;
    }
    case Opcode_load_integer:
    {
        if (state->read_level == state->frame_level) {
            AVMInteger_t value;
            state->stream->Read(&value);

            DEBUG_LOG("Load integer: %d", value);
            PushInt(value);
        } else {
            state->stream->Skip(sizeof(AVMInteger_t));
        }

        break;
    }
    case Opcode_load_float:
    {
        if (state->read_level == state->frame_level) {
            AVMFloat_t value;
            state->stream->Read(&value);

            DEBUG_LOG("Load float: %f", value);
            PushFloat(value);
        } else {
            state->stream->Skip(sizeof(AVMFloat_t));
        }

        break;
    }
    case Opcode_load_string:
    {
        int32_t len;
        state->stream->Read(&len);

        if (state->read_level == state->frame_level) {
            achar *str = new achar[len];
            state->stream->Read(str, len * sizeof(achar));

            DEBUG_LOG("Load string: %s", str);
            PushString(AVMString_t(str));

            delete[] str;
        } else {
            state->stream->Skip(len);
        }

        break;
    }
    case Opcode_load_null:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Load null");

            auto ref = Reference(*state->heap.AllocObject<Variable>());
            ref.Ref()->flags |= Object::FLAG_TEMPORARY;
            PushReference(ref);
        }

        break;
    }
    case Opcode_pop:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Pop stack");
            PopStack();
        }

        break;
    }
    case Opcode_unary_minus:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Unary -");
            Operation(&Variable::Negate);
        }

        break;
    }
    case Opcode_unary_not:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Unary !");
            Operation(&Variable::LogicalNot);
        }

        break;
    }
    case Opcode_add:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary +");
            Operation(&Variable::Add);
        }

        break;
    }
    case Opcode_sub:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary -");
            Operation(&Variable::Subtract);
        }

        break;
    }
    case Opcode_mul:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary *");
            Operation(&Variable::Multiply);
        }

        break;
    }
    case Opcode_div:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary /");
            Operation(&Variable::Divide);
        }

        break;
    }
    case Opcode_mod:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary %");
            Operation(&Variable::Modulus);
        }

        break;
    }
    case Opcode_pow:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary **");
            Operation(&Variable::Power);
        }

        break;
    }
    case Opcode_and:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary &&");
            Operation(&Variable::LogicalAnd);
        }

        break;
    }
    case Opcode_or:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary ||");
            Operation(&Variable::LogicalOr);
        }

        break;
    }
    case Opcode_eql:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary ==");
            Operation(&Variable::Equals);
        }

        break;
    }
    case Opcode_neql:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary !=");
            Operation(&Variable::NotEqual);
        }

        break;
    }
    case Opcode_less:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary <");
            Operation(&Variable::Less);
        }

        break;
    }
    case Opcode_greater:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary >");
            Operation(&Variable::Greater);
        }

        break;
    }
    case Opcode_less_eql:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary <=");
            Operation(&Variable::LessOrEqual);
        }

        break;
    }
    case Opcode_greater_eql:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary >=");
            Operation(&Variable::GreaterOrEqual);
        }

        break;
    }
    case Opcode_bit_and:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary &");
            Operation(&Variable::BitwiseAnd);
        }

        break;
    }
    case Opcode_bit_or:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary |");
            Operation(&Variable::BitwiseOr);
        }

        break;
    }
    case Opcode_bit_xor:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary &");
            Operation(&Variable::BitwiseXor);
        }

        break;
    }
    case Opcode_left_shift:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary <<");
            Operation(&Variable::LeftShift);
        }

        break;
    }
    case Opcode_right_shift:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary >>");
            Operation(&Variable::RightShift);
        }

        break;
    }
    case Opcode_assign:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary =");
            Assignment();
        }

        break;
    }
    case Opcode_add_assign:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary +=");
            Assignment(&Variable::Add);
        }

        break;
    }
    case Opcode_sub_assign:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary -=");
            Assignment(&Variable::Subtract);
        }

        break;
    }
    case Opcode_mul_assign:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary *=");
            Assignment(&Variable::Multiply);
        }

        break;
    }
    case Opcode_div_assign:
    {
        if (state->read_level == state->frame_level) {
            DEBUG_LOG("Binary /=");
            Assignment(&Variable::Divide);
        }

        break;
    }
    default:
    {
        auto last_pos = (((unsigned long)state->stream->Position()) - sizeof(Opcode_t));
        std::cout << "Unrecognized instruction '" << (int)opcode << "' at position: " << std::hex << last_pos << "\n";
        break;
    }
    }
}

void VMInstance::Execute(ByteStream *bs)
{
    state->stream = bs;

    while (state->stream->Position() < state->stream->Max()) {
        Opcode_t ins;
        state->stream->Read(&ins);
        HandleInstruction(ins);
    }
}
} // namespace avm