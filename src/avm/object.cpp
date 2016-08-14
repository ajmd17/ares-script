#include <detail/object.h>
#include <detail/vm_state.h>
#include <detail/reference.h>

#include <exception>

namespace avm {
void Object::AddFieldReference(const AVMString_t &name, Reference ref) {
  auto it = std::find_if(fields.begin(), fields.end(),
    [&](const std::pair<AVMString_t, Reference> &it) {
    return it.first == name;
  });
  if (it != fields.end()) {
    throw std::runtime_error("Member already exists");
  }
  fields.push_back(std::make_pair(name, ref));
}

Reference Object::GetFieldReference(const AVMString_t &name) {
  auto it = std::find_if(fields.begin(), fields.end(),
    [&](const std::pair<AVMString_t, Reference> &it) {
    return it.first == name;
  });
  if (it != fields.end()) {
    return it->second;
  } else {
    fields.push_back(std::make_pair(name, Reference()));
    return fields.back().second;
  }
}

Reference Object::GetFieldReference(size_t index) {
  return fields.at(index).second;
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