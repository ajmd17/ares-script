#include "Widget.h"

namespace arui {
Widget::Widget(Shader *shader, int x, int y, int w, int h) 
  : shader(shader), xpos(x), ypos(y), width(w), height(h) {
}

const std::string &Widget::GetText() const {
  return text;
}

void Widget::SetText(const std::string &str) {
  text = str;
}

Shader *Widget::GetShader() const {
  return shader;
}

int Widget::GetX() const {
  return xpos;
}

int Widget::GetY() const {
  return ypos;
}

void Widget::SetPosition(int x, int y) {
  xpos = x;
  ypos = y;
}

int Widget::GetWidth() const {
  return width;
}

int Widget::GetHeight() const {
  return height;
}

void Widget::SetSize(int w, int h) {
  width = w;
  height = h;
}
}