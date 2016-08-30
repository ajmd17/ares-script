#ifndef ARUI_SHADER_H
#define ARUI_SHADER_H
#include <string>
#include "Math/matrix4.h"

namespace arui {
class Shader {
public:
  Shader(const std::string &vertexSource, const std::string &fragmentSource);
  ~Shader();

  void Begin();
  void End();

  void SetUniformInt(const std::string &name, int value);
  void SetUniformFloat(const std::string &name, float value);
  void SetUniformFloat(const std::string &name, float x, float y);
  void SetUniformFloat(const std::string &name, float x, float y, float z);
  void SetUniformMatrix(const std::string &name, const Matrix4 &value);

private:
  unsigned int programID, vertexShaderID, fragmentShaderID;
  bool created = false;
};
}
#endif

