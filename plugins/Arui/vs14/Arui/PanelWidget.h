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

  virtual bool TestHover(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Hover(avm::VMState *state);
  virtual bool TestClick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Click(avm::VMState *state);
  virtual bool TestUnclick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Unclick(avm::VMState *state);
  virtual void Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety);

private:
  Mesh *mesh = nullptr;
  std::vector<Widget*> children;
};
}

#endif