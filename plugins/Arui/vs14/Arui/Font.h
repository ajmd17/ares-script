#ifndef ARUI_FONT_H
#define ARUI_FONT_H

#include "Shader.h"

#include <string>
#include <map>
#include <tuple>

namespace arui {
class Font {
public:
  Font(Shader *shader, const std::string &filepath);

  int GetLineWidth(const std::string &str) const;
  int GetLineHeight(const std::string &str) const;
  void DrawText(const std::string &str, double x, double y, double scale, 
    const std::tuple<float, float, float> &rgb);

private:

  void GenerateMesh();

  std::string filepath;

  struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    int width;
    int height;
    int bearingx;
    int bearingy;
    unsigned int Advance;    // Offset to advance to next glyph
    int horiAdvance;
  };

  std::map<char, Character> Characters;

  unsigned int VAO, VBO;
  Shader *shader;
};
}

#endif