#include <detail/vm_state.h>
#include <cstdio>

namespace avm {
VMState::VMState(VMInstance *vm)
    : vm(vm), 
      frame_level(AVM_LEVEL_GLOBAL), 
      read_level(AVM_LEVEL_GLOBAL),
      can_handle_exceptions(false),
      num_objects(0), 
      max_objects(GC_THRESHOLD_MIN), 
      max_heap_size(1000) /* in bytes */
{
    frames.push_back(new Frame());
    stack.reserve(100);
}

VMState::~VMState()
{
    // delete all frames and their objects by reverse looping
    int start = frame_level;
    while (start >= AVM_LEVEL_GLOBAL) {
        auto current = frames[start];
        if (current && (!current->locals.empty())) {
            for (long i = current->locals.size() - 1; i >= 0; i--) {
                current->locals[i].second.DeleteObject();
            }

            delete frames[start];
            frames[start] = nullptr;
        }

        --start;
    }
}

void VMState::HandleException(const Exception &except)
{
    frames[frame_level]->exception_occured = true;
    if (!can_handle_exceptions) {
        std::cout << "Unhandled exception: " << except.message << "\n";
        std::cout << "Type 'd' and press return to display memory dump\n";

        if (std::getchar() == (int)'d') {
            std::stringstream ss;
            ss << "Stack:\n";
            for (auto &&it : stack) {
                ss << "\t" << it.Ref()->ToString() << "\n";
            }
            ss << "\nHeap:\n";
            heap.DumpHeap(ss);

            ss << "\nFields:\n";
            for (size_t i = 0; i < frames.size(); i++) {
                ss << "#" << i << " {\n";
                Frame *frame = frames[i];
                for (size_t j = 0; j < frame->locals.size(); j++) {
                    ss << "\t#" << j << "\t" << frame->locals[j].first << "\n";
                }
                ss << "}\n";
            }

            std::cout << ss.str();
        }

        std::system("pause");
        std::exit(EXIT_FAILURE);
    }
}
}