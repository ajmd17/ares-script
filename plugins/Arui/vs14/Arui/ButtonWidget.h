#ifndef ARUI_BUTTON_WIDGET_H
#define ARUI_BUTTON_WIDGET_H

#include "Widget.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

#include <string>
#include <tuple>
#include <functional>

namespace arui {
enum ButtonState {
  ButtonState_normal,
  ButtonState_hover,
  ButtonState_clicked
};

class ButtonWidget : public Widget {
public:
  ButtonWidget(const std::string &text, const std::tuple<float, float, float> &textcolor,
    Texture *texture, Texture *texture_hover, Texture *texture_clicked,
    Shader *shader, int x, int y, int w, int h);
  ~ButtonWidget();

  std::string GetText() const;
  void SetText(const std::string &str);
  std::tuple<float, float, float> GetTextColor() const;
  void SetTextColor(const std::tuple<float, float, float> &rgb);

  virtual bool TestHover(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Hover(avm::VMState *state);
  virtual bool TestClick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Click(avm::VMState *state);
  virtual bool TestUnclick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety);
  virtual void Unclick(avm::VMState *state);
  virtual void Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety);

private:
  double blendtime;

  ButtonState btnstate, prevstate;

  std::string text;
  std::tuple<float, float, float> textcolor;
  Texture *texture = nullptr;
  Texture *texture_hover = nullptr;
  Texture *texture_clicked = nullptr;
  Mesh *mesh = nullptr;
};
}

#endif