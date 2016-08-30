#include "LabelWidget.h"

namespace arui {
LabelWidget::LabelWidget(const std::string &text, const std::tuple<float, float, float> &textcolor,
  int x, int y)
  :
  text(text), textcolor(textcolor),
  Widget(shader, x, y, 20, 10) {
}

LabelWidget::~LabelWidget() {
}

std::string LabelWidget::GetText() const {
  return text;
}

void LabelWidget::SetText(const std::string &str) {
  text = str;
}

std::tuple<float, float, float> LabelWidget::GetTextColor() const {
  return textcolor;
}

void LabelWidget::SetTextColor(const std::tuple<float, float, float> &rgb) {
  textcolor = rgb;
}

bool LabelWidget::TestHover(int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;
  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      Hover();
      return true;
    }
  }
  return false;
}

void LabelWidget::Hover() {
  std::printf("Label hover\n");
}

bool LabelWidget::TestClick(int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;
  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      Click();
      return true;
    }
  }
  return false;
}

void LabelWidget::Click() {
  std::printf("Label clicked!\n");
}

bool LabelWidget::TestUnclick(int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;
  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      Unclick(); // trigger click events if released within button
      return true;
    }
  }
  return false;
}

void LabelWidget::Unclick() {
  std::printf("Label unclicked!\n");

  // perform action here
}

void LabelWidget::Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety - ypos;

  font->DrawText(text, absx, absy, 1.0, textcolor);
}
}