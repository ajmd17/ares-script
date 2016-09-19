#include "PanelWidget.h"
#include "Quad.h"
#include "Math/matrix4.h"
#include "Math/matrix_util.h"
#include "Math/vector3.h"

#include <GL/glew.h>

namespace arui {
PanelWidget::PanelWidget(Shader *shader, int x, int y, int w, int h)
  : Widget(shader, x, y, w, h) {
  mesh = Quad::CreateQuad();
}

PanelWidget::~PanelWidget() {
  if (mesh) {
    delete mesh;
  }

  for (auto &&child : children) {
    if (child != nullptr) {
      delete child;
      child = nullptr;
    }
  }
}

void PanelWidget::AddWidget(Widget *widget) {
  children.push_back(widget);
}

bool PanelWidget::TestHover(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;

  // check all children first, as they are above this widget
  for (auto &&child : children) {
    if (child != nullptr) {
      if (child->TestHover(state, mouse_x, mouse_y, absx, absy)) {
        return false; // this wasn't hit
      }
    }
  }

  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      // no children were clicked, so click this
      Hover(state);
      return true;
    }
  }
  return false;
}

void PanelWidget::Hover(avm::VMState *state) {
}

bool PanelWidget::TestClick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;

  // check all children first, as they are above this widget
  for (auto &&child : children) {
    if (child != nullptr) {
      if (child->TestClick(state, mouse_x, mouse_y, absx, absy)) {
        return false; // this wasn't hit
      }
    }
  }

  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      // no children were clicked, so click this
      Click(state);
      return true;
    }
  }
  return false;
}

void PanelWidget::Click(avm::VMState *state) {
}

bool PanelWidget::TestUnclick(avm::VMState *state, int mouse_x, int mouse_y, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety + ypos;

  // check all children first, as they are above this widget
  for (auto &&child : children) {
    if (child != nullptr) {
      if (child->TestUnclick(state, mouse_x, mouse_y, absx, absy)) {
        return false; // this wasn't hit
      }
    }
  }

  if (mouse_x >= absx && mouse_x <= absx + width) {
    if (mouse_y >= absy && mouse_y <= absy + height) {
      // no children were clicked, so click this
      Unclick(state);
      return true;
    }
  }
  return false;
}

void PanelWidget::Unclick(avm::VMState *state) {
}

void PanelWidget::Draw(const Matrix4 &proj, Font *font, int offsetx, int offsety) {
  int absx = offsetx + xpos;
  int absy = offsety - ypos;

  if (mesh) {
    shader->Begin();
    // apply matrices to shader
    shader->SetUniformMatrix("u_projMatrix", proj);

    Matrix4 trans_mat, scale_mat;
    MatrixUtil::ToTranslation(trans_mat, Vector3(absx, absy - height, 0));
    MatrixUtil::ToScaling(scale_mat, Vector3(width, height, 1));
    Matrix4 transform = scale_mat * trans_mat;
    shader->SetUniformMatrix("u_modelMatrix", transform);

    mesh->Draw();

    shader->End();
  }

  for (auto &&child : children) {
    if (child != nullptr) {
      child->Draw(proj, font, absx, absy);
    }
  }
}
}