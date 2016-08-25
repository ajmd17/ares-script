#include <detail/frame.h>

namespace avm {
Frame::Frame()
  : last_cond(false), exception_occured(false) {
}

bool Frame::GetLocal(const AVMString_t &name, Reference &out) {
  auto elt = std::find_if(locals.begin(), locals.end(),
    [&name](const std::pair<AVMString_t, Reference> &element) {
    return element.first == name;
  });

  if (elt != locals.end()) {
    out = elt->second;
    return true;
  }

  return false;
}
} // namespace avm