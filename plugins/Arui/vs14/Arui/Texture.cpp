#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GL/glew.h>

namespace arui {
Texture::Texture(const std::string &path) {
  int width, height, comp;
  unsigned char *bytes = stbi_load(path.c_str(), &width, &height, &comp, STBI_rgb_alpha);

  if (bytes == nullptr) {
    throw "Could not load texture";
  }


  glGenTextures(1, &id);

  Use();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
    width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

  glGenerateMipmap(GL_TEXTURE_2D);

  End();

  stbi_image_free(bytes);
}

Texture::~Texture() {
  glDeleteTextures(1, &id);
}

void Texture::Use() {
  glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::End() {
  glBindTexture(GL_TEXTURE_2D, 0);
}
}