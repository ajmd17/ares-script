#include <detail/object.h>
#include <detail/vm_state.h>
#include <detail/reference.h>

#include <exception>

namespace avm {
bool Object::AddFieldReference(VMState *state, const AVMString_t &name, Reference ref) {
  auto it = std::find_if(fields.begin(), fields.end(),
    [&](const std::pair<AVMString_t, Reference> &it) {
    return it.first == name;
  });
  if (it != fields.end()) {
    throw std::runtime_error("Member already exists");
    return false;
  }
  fields.push_back(std::make_pair(name, ref));
  return true;
}

bool Object::GetFieldReference(VMState *state, const AVMString_t &name, Reference &out) {
  auto it = std::find_if(fields.begin(), fields.end(),
    [&](const std::pair<AVMString_t, Reference> &it) {
    return it.first == name;
  });
  if (it != fields.end()) {
    out = it->second;
    return true;
  } else {
    state->HandleException(MemberNotFoundException(name));
    return false;
  }
}

bool Object::GetFieldReference(VMState *state, size_t index, Reference &out) {
  if (index < fields.size()) {
    out = fields[index].second;
    return true;
  } else {
    return false;
  }
}

void Object::Mark() {
  if (!(flags & FLAG_MARKED)) {
    flags |= FLAG_MARKED;
    MarkFields();
  }
}

void Object::MarkFields() {
  for (auto &&member : fields) {
    member.second.Ref()->Mark();
  }
}
} // namespace avm