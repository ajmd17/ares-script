#ifndef ARUI_PANEL_WIDGET_H
#define ARUI_PANEL_WIDGET_H

#include "Widget.h"
#include "Mesh.h"

namespace arui {
class PanelWidget : public Widget {
public:
  PanelWidget(Shader *shader, int x, int y, int w, int h);
  ~PanelWidget();

  void AddWidget(Widget *widget);

  virtual bool TestHover(int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Hover();
  virtual bool TestClick(int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Click();
  virtual bool TestUnclick(int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Unclick();
  virtual void Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety);

private:
  Mesh *mesh = nullptr;
  std::vector<Widget*> children;
};
}

#endif