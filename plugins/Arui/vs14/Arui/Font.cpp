#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>

#include <iostream>

namespace arui {
Font::Font(Shader *shader, const std::string &filepath)
  : shader(shader), filepath(filepath) {
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    std::cout << "Could not initialize FreeType\n";
    return;
  }

  FT_Face face;
  if (FT_New_Face(ft, filepath.c_str(), 0, &face)) {
    std::cout << "Failed to load font\n";
    return;
  }

  FT_Select_Charmap(face, ft_encoding_unicode);
  FT_Set_Pixel_Sizes(face, 0, 14);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

  for (GLubyte c = 0; c < 128; c++) {
    // Load character glyph
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "Failed to load Glyph '" << c << "'\n";
      continue;
    }

    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
      face->glyph->bitmap.width, face->glyph->bitmap.rows, 0,
      GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character character {
      texture,
      face->glyph->bitmap.width,
      face->glyph->bitmap.rows,
      face->glyph->bitmap_left,
      face->glyph->bitmap_top,
      face->glyph->advance.x
    };

    Characters.insert({ c, character });
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  GenerateMesh();
}

int Font::GetLineWidth(const std::string &str) const {
  int lw = 0;
  for (char ch : str) {
    lw += Characters.at(ch).Advance >> 6;
  }
  return lw;
}

int Font::GetLineHeight(const std::string &str) const {
  int lh = 0;
  for (char ch : str) {
    Character character = Characters.at(ch);
    if (character.height > lh) {
      lh = character.height;
    }
  }
  return lh;
}

void Font::GenerateMesh() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Font::DrawText(const std::string &str, double x, double y, double scale, 
  const std::tuple<float, float, float> &rgb) {

  // Activate corresponding render state
  shader->Begin();

  glActiveTexture(GL_TEXTURE0);
  shader->SetUniformInt("text", 0);

  shader->SetUniformFloat("u_color", std::get<0>(rgb), std::get<1>(rgb), std::get<2>(rgb));

  glBindVertexArray(VAO);

  int lineheight = GetLineHeight(str);

  // Iterate through all characters
  for (auto c = str.begin(); c != str.end(); ++c) {
    Character ch = Characters[*c];

    GLfloat xpos = x + ch.bearingx * scale;
    GLfloat ypos = y - (ch.height - ch.bearingy) * scale;

    GLfloat w = ch.width * scale;
    GLfloat h = ch.height * scale;
    // Update VBO for each character
    GLfloat box[6][4] = {
      { xpos,     ypos + h - lineheight,   0.0, 0.0 },
      { xpos,     ypos     - lineheight,   0.0, 1.0 },
      { xpos + w, ypos     - lineheight,   1.0, 1.0 },

      { xpos,     ypos + h - lineheight,   0.0, 0.0 },
      { xpos + w, ypos     - lineheight,   1.0, 1.0 },
      { xpos + w, ypos + h - lineheight,   1.0, 0.0 }
    };

    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(box), box);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}
}