#include "Arui.h"
#include "PanelWidget.h"
#include "ButtonWidget.h"
#include "LabelWidget.h"
#include "Shader.h"
#include "Font.h"
#include "Texture.h"
#include "Math/matrix_util.h"

#include <detail/variable.h>
#include <detail/function.h>
#include <detail/check_args.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

using namespace arui;

Shader *panel_shader = nullptr;
Shader *button_shader = nullptr;
Shader *text_shader = nullptr;

Font *font = nullptr;

Texture *btn_texture = nullptr;
Texture *btn_texture_hover = nullptr;
Texture *btn_texture_clicked = nullptr;

Matrix4 model_mat;
Matrix4 view_mat;
Matrix4 proj_mat;

double mouse_x = 0, mouse_y = 0;
int window_width = 0, window_height = 0;

std::vector<Widget*> widgets;
avm::VMState *avm_state = nullptr;

static GLFWwindow *window = nullptr;
std::thread render_thread;
std::mutex mtx;

std::queue<std::function<void()>> _queue; // once gl starts these will run

bool can_render = true;
bool opengl_init = false;

void Render();

static void ErrorCallback(int error, const char *description) {
  std::cout << "Arui Error: " << description << "\n";
}

static void WindowResizedCallback(GLFWwindow *window, int width, int height) {
  std::printf("resize %d %d\n", width, height);

  if (window_width != width) {
    // width resized
    int diff = window_width - width;
    mtx.lock();
    for (auto &&widget : widgets) {
      if (widget != nullptr) {
        if (widget->anchor == Anchor_none) {
          widget->SetPosition(widget->GetX() - diff, widget->GetY());
        } else {
          if (widget->anchor & Anchor_left) {
            widget->SetSize(widget->GetWidth() - diff, widget->GetHeight());
          }

          if (widget->anchor & Anchor_right) {
            //widget->SetPosition(widget->GetX() + diff, widget->GetY());
            widget->SetSize(widget->GetWidth() - diff, widget->GetHeight());
          }
        }
      }
    }
    mtx.unlock();
  }
  window_width = width;

  if (window_height != height) {
    // height resized
    int diff = window_height - height;
    mtx.lock();
    for (auto &&widget : widgets) {
      if (widget != nullptr) {
        if (widget->anchor == Anchor_none) {
          widget->SetPosition(widget->GetX(), widget->GetY() - diff);
        } else {
        }
      }
    }
    mtx.unlock();
  }
  window_height = height;

  // Call Render() upon resize.
  Render();
}

static void MouseMoveCallback(GLFWwindow *window, double xpos, double ypos) {
  // test hover
  mtx.lock();
  for (auto &&widget : widgets) {
    if (widget != nullptr) {
      widget->TestHover(avm_state, (int)xpos, (int)ypos, 0, 0);
    }
  }
  mtx.unlock();
}

static void MouseClickCallback(GLFWwindow* window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    // test click
    mtx.lock();
    for (auto &&widget : widgets) {
      if (widget != nullptr) {
        widget->TestClick(avm_state, (int)mouse_x, (int)mouse_y, 0, 0);
      }
    }
    mtx.unlock();
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    // test released
    mtx.lock();
    for (auto &&widget : widgets) {
      if (widget != nullptr) {
        widget->TestUnclick(avm_state, (int)mouse_x, (int)mouse_y, 0, 0);
      }
    }
    mtx.unlock();
  }
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

static void Render() {
  glViewport(0, 0, window_width, window_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glfwGetCursorPos(window, &mouse_x, &mouse_y);

  view_mat = Matrix4::Identity();
  MatrixUtil::ToOrtho(proj_mat, 0, window_width, 0, window_height, -1.0f, 1.0f);

  // lock mutex
  mtx.lock();

  // apply projection matrix to font shader
  text_shader->Begin();
  text_shader->SetUniformMatrix("u_projMatrix", proj_mat);
  text_shader->End();

  // render widgets
  for (auto &&widget : widgets) {
    if (widget != nullptr) {
      widget->Draw(proj_mat, font, 0, window_height);
    }
  }

  mtx.unlock();

  glfwSwapBuffers(window);
  glfwPollEvents();
}

static void Loop() {
  glfwMakeContextCurrent(window);
  glfwGetWindowSize(window, &window_width, &window_height);

  while (!glfwWindowShouldClose(window) && can_render) {
    Render();

    // sleep thread
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  for (auto &&widget : widgets) {
    if (widget != nullptr) {
      delete widget;
      widget = nullptr;
    }
  }

  if (button_shader != nullptr) {
    delete button_shader;
    button_shader = nullptr;
  }
  if (panel_shader != nullptr) {
    delete panel_shader;
    panel_shader = nullptr;
  }
  if (text_shader != nullptr) {
    delete text_shader;
    text_shader = nullptr;
  }
  if (font != nullptr) {
    delete font;
    font = nullptr;
  }
  if (btn_texture != nullptr) {
    delete btn_texture;
    btn_texture = nullptr;
  }
  if (btn_texture_hover != nullptr) {
    delete btn_texture_hover;
    btn_texture_hover = nullptr;
  }
  if (btn_texture_clicked != nullptr) {
    delete btn_texture_clicked;
    btn_texture_clicked = nullptr;
  }
}

static bool CreateGlfwWindow() {
  // init GLFW window
  glfwSetErrorCallback(ErrorCallback);

  if (!glfwInit()) {
    return false;
  }

  window = glfwCreateWindow(512, 256, "Arui", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return false;
  }

  glfwSetWindowSizeCallback(window, WindowResizedCallback);
  glfwSetCursorPosCallback(window, MouseMoveCallback);
  glfwSetMouseButtonCallback(window, MouseClickCallback);
  glfwSetKeyCallback(window, KeyCallback);
  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    throw std::exception("error initializing glew");
  }

  glfwSwapInterval(1);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // create button shader
  {
    std::string vert_src =
      "uniform mat4 u_modelMatrix;"
      "uniform mat4 u_viewMatrix;"
      "uniform mat4 u_projMatrix;"
      "varying vec3 vertex;"
      "varying vec2 texcoord;"
      "void main() {"
      "vertex = gl_Vertex.xyz;"
      "texcoord = gl_MultiTexCoord0.st;"
      "gl_Position = u_projMatrix * u_modelMatrix * gl_Vertex;"
      "}";
    std::string frag_src =
      "varying vec3 vertex;"
      "varying vec2 texcoord;"
      "uniform sampler2D u_texture;"
      "uniform sampler2D u_texture_hover;"
      "uniform sampler2D u_texture_clicked;"
      "uniform int u_state;"
      "uniform int u_prevstate;"
      "uniform float u_blendtime;"

      // function to get texture from state
      "vec4 GetTex(int state) {"
      "if (state == 0) {"
      "return  texture2D(u_texture, texcoord);"
      "} else if (state == 1) {"
      "return  texture2D(u_texture_hover, texcoord);"
      "} else {"
      "return  texture2D(u_texture_clicked, texcoord);"
      "}"
      "}"

      "void main() {"
      "vec4 current = GetTex(u_state);"
      "vec4 prev = GetTex(u_prevstate);"
      "vec4 finaltex = mix(prev, current, vec4(u_blendtime));"
      "gl_FragColor = finaltex;"
      "}";
    button_shader = new Shader(vert_src, frag_src);
  }

  // create panel shader
  {
    std::string vert_src =
      "uniform mat4 u_modelMatrix;"
      "uniform mat4 u_viewMatrix;"
      "uniform mat4 u_projMatrix;"
      "varying vec3 vertex;"
      "varying vec2 texcoord;"
      "void main() {"
      "vertex = gl_Vertex.xyz;"
      "texcoord = gl_MultiTexCoord0.st;"
      "gl_Position = u_projMatrix * u_modelMatrix * gl_Vertex;"
      "}";
    std::string frag_src =
      "varying vec3 vertex;"
      "varying vec2 texcoord;"
      "uniform sampler2D u_texture;"

      "void main() {"
      "gl_FragColor = vec4(1.0);"
      "}";
    panel_shader = new Shader(vert_src, frag_src);
  }

  // create text shader
  {
    std::string vert_src =
      "uniform mat4 u_modelMatrix;"
      "uniform mat4 u_viewMatrix;"
      "uniform mat4 u_projMatrix;"
      "varying vec2 texcoord;"
      "void main() {"
      "texcoord = gl_Vertex.zw;"
      "gl_Position = u_projMatrix * vec4(gl_Vertex.xy, 0.0, 1.0);"
      "}";
    std::string frag_src =
      "uniform sampler2D text;"
      "uniform vec3 u_color;"
      "varying vec2 texcoord;"
      "void main() {"
      "gl_FragColor = vec4(u_color, texture2D(text, vec2(texcoord.x, texcoord.y)).r);"
      "}";
    text_shader = new Shader(vert_src, frag_src);

    font = new Font(text_shader, "gui/font/DejaVuSans.ttf");
  }

  btn_texture = new Texture("gui/img/button.png");
  btn_texture_hover = new Texture("gui/img/button_hover.png");
  btn_texture_clicked = new Texture("gui/img/button_clicked.png");

  widgets.reserve(20);

  // create test widget
  auto *main_panel = new PanelWidget(panel_shader, 25, 25, 256, 256);

  auto *btn1 = new ButtonWidget("Click Me", std::make_tuple(1.0f, 1.0f, 1.0f),
    btn_texture, btn_texture_hover, btn_texture_clicked,
    button_shader, 12, 12, 128, 32);
  btn1->anchor = Anchor_right;
  widgets.push_back(btn1);

  /*main_panel->AddWidget(new ButtonWidget("Squeeky Deeky", std::make_tuple(1.0f, 1.0f, 1.0f),
    btn_texture, btn_texture_hover, btn_texture_clicked,
    button_shader, 24, 64, 128, 32));

  main_panel->AddWidget(new LabelWidget("Weinerschnizel", std::make_tuple(0.0f, 0.0f, 0.0f), 52, 120));

  widgets.push_back(main_panel);*/

  // enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  mtx.lock();
  opengl_init = true;

  // now that opengl is initialized, we can execute the queued functions
  while (!_queue.empty()) {
    _queue.front()();
    _queue.pop();
  }

  mtx.unlock();

  Loop();

  return true;
}

ARUI_API void InitArui(avm::VMState *state, avm::Object **args, uint32_t argc) {
  if (avm::CheckArgs(state, 0, argc)) {
    avm_state = state;

    render_thread = std::thread(CreateGlfwWindow);

    auto ref = avm::Reference(*state->heap.AllocObject<avm::Variable>());
    ref.Ref()->flags |= avm::Object::FLAG_TEMPORARY;
    state->stack.push_back(ref);
  }
}

ARUI_API void DestroyArui(avm::VMState *state, avm::Object **args, uint32_t argc) {
  if (avm::CheckArgs(state, 0, argc)) {
    can_render = false;
    render_thread.join();

    auto ref = avm::Reference(*state->heap.AllocObject<avm::Variable>());
    ref.Ref()->flags |= avm::Object::FLAG_TEMPORARY;
    state->stack.push_back(ref);
  }
}

ARUI_API void AddButton(avm::VMState *state, avm::Object **args, uint32_t argc) {
  std::cout << "Call AddButton()\n";

  /* args:
      button texts
      button x
      button y
      callback function
  */
  if (avm::CheckArgs(state, 4, argc)) {
    bool good = true;

    avm::Variable *text = dynamic_cast<avm::Variable*>(args[0]);
    if (text == nullptr) {
      state->HandleException(avm::TypeException(args[0]->TypeString()));
      good = false;
    }

    avm::Variable *btnx = dynamic_cast<avm::Variable*>(args[1]);
    if (btnx == nullptr) {
      state->HandleException(avm::TypeException(args[1]->TypeString()));
      good = false;
    }

    avm::Variable *btny = dynamic_cast<avm::Variable*>(args[2]);
    if (btny == nullptr) {
      state->HandleException(avm::TypeException(args[2]->TypeString()));
      good = false;
    }

    avm::Func *callback = dynamic_cast<avm::Func*>(args[3]);
    if (callback == nullptr) {
      state->HandleException(avm::TypeException(args[3]->TypeString()));
      good = false;
    }

    if (good) {
      std::string text_str;
      int btnx_i, btny_i;

      if (text->type == avm::Variable::Type_string) {
        text_str = text->Cast<avm::AVMString_t>();
      } else {
        state->HandleException(avm::ConversionException(text->TypeString(), "string"));
        good = false;
      }

      if (btnx->type == avm::Variable::Type_int) {
        btnx_i = btnx->Cast<avm::AVMInteger_t>();
      } else {
        state->HandleException(avm::ConversionException(btnx->TypeString(), "int"));
        good = false;
      }

      if (btny->type == avm::Variable::Type_int) {
        btny_i = btny->Cast<avm::AVMInteger_t>();
      } else {
        state->HandleException(avm::ConversionException(btny->TypeString(), "int"));
        good = false;
      }

      if (good) {
        mtx.lock();

        std::string callbackname = CreateCallbackName();
        auto cb = callback->Clone(state);
        // add global so that callback does not expire.
        state->frames[0]->locals.push_back({ callbackname, cb });
        std::cout << "add global: " << callbackname << "\n";

        auto lambda = [=]() {
          auto *btn = new ButtonWidget(text_str, std::make_tuple(1.0f, 1.0f, 1.0f),
            btn_texture, btn_texture_hover, btn_texture_clicked,
            button_shader, btnx_i, btny_i, 128, 32);

          state->frames[0]->GetLocal(callbackname, btn->callback);
          widgets.push_back(btn);
        };

        if (!opengl_init) {
          std::cout << "opengl not yet init\n";
          _queue.push(lambda);
        } else {
          std::cout << "opengl is init\n";
          lambda();
        }
        mtx.unlock();

        /// \todo : make it push an object that represents the widget with member functions
        auto ref = avm::Reference(*state->heap.AllocObject<avm::Variable>());
        ref.Ref()->flags |= avm::Object::FLAG_TEMPORARY;
        state->stack.push_back(ref);
      }
    }
  }
}

std::string CreateCallbackName() {
  static int gen = 0;
  std::string res = std::string("$arui_callback") + std::to_string(gen++);
  return res;
}