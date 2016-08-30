#ifndef ARES_LABEL_WIDGET_H
#define ARES_LABEL_WIDGET_H

#include "Widget.h"

#include <tuple>

namespace arui {
class LabelWidget : public Widget {
public:
  LabelWidget(const std::string &text, const std::tuple<float, float, float> &textcolor, 
    int x, int y);
  ~LabelWidget();

  std::string GetText() const;
  void SetText(const std::string &str);
  std::tuple<float, float, float> GetTextColor() const;
  void SetTextColor(const std::tuple<float, float, float> &rgb);

  virtual bool TestHover(int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Hover();
  virtual bool TestClick(int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Click();
  virtual bool TestUnclick(int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Unclick();
  virtual void Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety);

private:
  std::string text;
  std::tuple<float, float, float> textcolor;
};
}

#endif