#include "ButtonWidget.h"
#include "Quad.h"
#include "Math/matrix4.h"
#include "Math/matrix_util.h"
#include "Math/vector3.h"

#include <GL/glew.h>

#include <cmath>

namespace arui {
ButtonWidget::ButtonWidget(const std::string &text, const std::tuple<float, float, float> &textcolor,
  Texture *texture, Texture *texture_hover, Texture *texture_clicked,
  Shader *shader, int x, int y, int w, int h)
  : btnstate(ButtonState_normal), prevstate(ButtonState_normal), blendtime(0.0),
  text(text), textcolor(textcolor),
  texture(texture), texture_hover(texture_hover), texture_clicked(texture_clicked),
  Widget(shader, x, y, w, h) {
  mesh = Quad::CreateQuad();
}

ButtonWidget::~ButtonWidget() {
  if (mesh) {
    delete mesh;
  }
}

std::string ButtonWidget::GetText() const {
  return text;
}

void ButtonWidget::SetText(const std::string &str) {
  text = str;
}

std::tuple<float, float, float> ButtonWidget::GetTextColor() const {
  return textcolor;
}

void ButtonWidget::SetTextColor(const std::tuple<float, float, float> &rgb) {
  textcolor = rgb;
}

bool ButtonWidget::TestHover(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;
  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      // if we button is clicked or already hovered over, do nothing
      if (btnstate == ButtonState_normal) {
        prevstate = btnstate;
        btnstate = ButtonState_hover;
        blendtime = 0.0;
        Hover(state);
      }
      return true;
    }
  }

  if (btnstate == ButtonState_hover) {
    prevstate = btnstate;
    btnstate = ButtonState_normal;
    blendtime = 0.0;
  }
  return false;
}

void ButtonWidget::Hover(avm::VMState *state) {
}

bool ButtonWidget::TestClick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;
  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      prevstate = btnstate;
      btnstate = ButtonState_clicked;
      blendtime = 0.0;
      Click(state);
      return true;
    }
  }
  return false;
}

void ButtonWidget::Click(avm::VMState *state) {
}

bool ButtonWidget::TestUnclick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;
  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      prevstate = btnstate;
      btnstate = ButtonState_normal;
      blendtime = 0.0;
      Unclick(state); // trigger click events if released within button
      return true;
    }
  }

  if (btnstate == ButtonState_clicked) {
    prevstate = btnstate;
    btnstate = ButtonState_normal;
    blendtime = 0.0;
    return true;
  }

  return false;
}

void ButtonWidget::Unclick(avm::VMState *state) {
  // perform action here
  if (callback.Ptr()) {
    callback.Ref()->invoke(state, 0);
  }
}

void ButtonWidget::Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety - ypos;

  if (mesh) {
    if (blendtime < 1.0) {
      blendtime += 0.3;
    }

    shader->Begin();
    // apply matrices to shader
    shader->SetUniformMatrix("u_projMatrix", proj);

    Matrix4 trans_mat, scale_mat;
    MatrixUtil::ToTranslation(trans_mat, Vector3(absx, absy - height, 0));
    MatrixUtil::ToScaling(scale_mat, Vector3(width, height, 1));
    Matrix4 transform = scale_mat * trans_mat;
    shader->SetUniformMatrix("u_modelMatrix", transform);

    glActiveTexture(GL_TEXTURE0);
    texture->Use();
    shader->SetUniformInt("u_texture", 0);

    glActiveTexture(GL_TEXTURE1);
    texture_hover->Use();
    shader->SetUniformInt("u_texture_hover", 1);

    glActiveTexture(GL_TEXTURE2);
    texture_clicked->Use();
    shader->SetUniformInt("u_texture_clicked", 2);

    shader->SetUniformInt("u_state", (int)btnstate);
    shader->SetUniformInt("u_prevstate", (int)prevstate);
    shader->SetUniformFloat("u_blendtime", std::fminf((float)blendtime, 1.0));

    mesh->Draw();

    texture->End();
    shader->End();

    // draw text on top of button
    font->DrawText(text, absx - (font->GetLineWidth(text) / 2) + (width / 2),
      absy - (height / 2) + (font->GetLineHeight(text) / 2), 1.0, textcolor);
  }
}
}