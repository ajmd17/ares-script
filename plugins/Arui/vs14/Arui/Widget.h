#ifndef ARUI_WIDGET_H
#define ARUI_WIDGET_H

#include "Shader.h"
#include "Font.h"

#include <avm/detail/object.h>
#include <avm/detail/vm_state.h>

#include <string>

namespace arui {

enum Anchor {
  Anchor_none = 0x00,
  Anchor_left = 0x01,
  Anchor_right = 0x02,
  Anchor_top = 0x04,
  Anchor_bottom = 0x08
};

class Widget {
public:
  int anchor = Anchor_none;
  avm::Reference callback;

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

  virtual bool TestHover(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) = 0;
  virtual void Hover(avm::VMState *state) = 0;
  virtual bool TestClick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) = 0;
  virtual void Click(avm::VMState *state) = 0;
  virtual bool TestUnclick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) = 0;
  virtual void Unclick(avm::VMState *state) = 0;
  virtual void Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety) = 0;

protected:
  std::string text;
  Shader *shader = nullptr;
  int xpos, ypos, width, height;
};
}

#endif