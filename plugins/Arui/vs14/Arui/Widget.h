#ifndef ARUI_WIDGET_H
#define ARUI_WIDGET_H

#include "Shader.h"
#include "Font.h"

#include <string>

namespace arui {
class Widget {
public:
  Widget(Shader *shader, int x, int y, int w, int h);
  virtual ~Widget() = default;

  const std::string &GetText() const;
  void SetText(const std::string &str);
  Shader *GetShader() const;

  int GetX() const;
  int GetY() const;
  void SetPosition(int x, int y);
  int GetWidth() const;
  int GetHeight() const;
  void SetSize(int w, int h);

  virtual bool TestHover(int mouse_x, int mouse_y, int offsetx, int offsety) = 0;
  virtual void Hover() = 0;
  virtual bool TestClick(int mouse_x, int mouse_y, int offsetx, int offsety) = 0;
  virtual void Click() = 0;
  virtual bool TestUnclick(int mouse_x, int mouse_y, int offsetx, int offsety) = 0;
  virtual void Unclick() = 0;
  virtual void Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety) = 0;

protected:
  std::string text;
  Shader *shader = nullptr;
  int xpos, ypos, width, height;
};
}

#endif