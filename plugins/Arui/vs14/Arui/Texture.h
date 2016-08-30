#ifndef ARUI_TEXTURE_H
#define ARUI_TEXTURE_H

#include <string>

namespace arui {
class Texture {
public:
  Texture(const std::string &path);
  ~Texture();

  void Use();
  void End();

protected:
  unsigned int id;
};
}

#endif