#include "Shader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

namespace arui {
Shader::Shader(const std::string &vertexSource, const std::string &fragmentSource) {
  // Create an empty vertex shader handle
  vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

  //Send the vertex shader source code to GL
  //Note that std::string's .c_str is NULL character terminated.
  const GLchar *source = (const GLchar *)vertexSource.c_str();
  glShaderSource(vertexShaderID, 1, &source, 0);

  //Compile the vertex shader
  glCompileShader(vertexShaderID);

  GLint isCompiled = 0;
  glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    char *infoLog = new char[maxLength];
    glGetShaderInfoLog(vertexShaderID, maxLength, &maxLength, &infoLog[0]);

    std::cout << "Shader error: " << infoLog << "\n";

    delete[] infoLog;

    //We don't need the shader anymore.
    glDeleteShader(vertexShaderID);

    //Use the infoLog as you see fit.

    //In this simple program, we'll just leave
    return;
  }

  //Create an empty fragment shader handle
  fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  //Send the fragment shader source code to GL
  //Note that std::string's .c_str is NULL character terminated.
  source = (const GLchar *)fragmentSource.c_str();
  glShaderSource(fragmentShaderID, 1, &source, 0);

  //Compile the fragment shader
  glCompileShader(fragmentShaderID);

  glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    char *infoLog = new char[maxLength];
    glGetShaderInfoLog(fragmentShaderID, maxLength, &maxLength, &infoLog[0]);

    std::cout << "Shader error: " << infoLog << "\n";

    delete[] infoLog;

    //We don't need the shader anymore.
    glDeleteShader(fragmentShaderID);
    //Either of them. Don't leak shaders.
    glDeleteShader(vertexShaderID);

    //Use the infoLog as you see fit.

    //In this simple program, we'll just leave
    return;
  }

  //Vertex and fragment shaders are successfully compiled.
  //Now time to link them together into a program.
  //Get a program object.
  programID = glCreateProgram();

  //Attach our shaders to our program
  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);

  //Link our program
  glLinkProgram(programID);

  //Note the different functions here: glGetProgram* instead of glGetShader*.
  GLint isLinked = 0;
  glGetProgramiv(programID, GL_LINK_STATUS, (int *)&isLinked);
  if (isLinked == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    char *infoLog = new char[maxLength];
    glGetProgramInfoLog(programID, maxLength, &maxLength, &infoLog[0]);

    std::cout << "Shader error: " << infoLog << "\n";

    delete[] infoLog;

    //We don't need the program anymore.
    glDeleteProgram(programID);
    //Don't leak shaders either.
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    //Use the infoLog as you see fit.

    //In this simple program, we'll just leave
    return;
  }

  //Always detach shaders after a successful link.
  glDetachShader(programID, vertexShaderID);
  glDetachShader(programID, fragmentShaderID);

  created = true;
}


Shader::~Shader() {
  if (created) {
    //We don't need the program anymore.
    glDeleteProgram(programID);
    //Don't leak shaders either.
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
  }
}

void Shader::Begin() {
  glUseProgram(programID);
}

void Shader::End() {
  glUseProgram(0);
}

void Shader::SetUniformInt(const std::string &name, int value) {
  auto loc = glGetUniformLocation(programID, name.c_str());
  if (loc == -1) {
    throw "uniform not found";
  }
  glProgramUniform1i(programID, loc, value);
}

void Shader::SetUniformFloat(const std::string &name, float value) {
  auto loc = glGetUniformLocation(programID, name.c_str());
  if (loc == -1) {
    throw "uniform not found";
  }
  glProgramUniform1f(programID, loc, value);
}

void Shader::SetUniformFloat(const std::string &name, float x, float y) {
  auto loc = glGetUniformLocation(programID, name.c_str());
  if (loc == -1) {
    throw "uniform not found";
  }
  glProgramUniform2f(programID, loc, x, y);
}

void Shader::SetUniformFloat(const std::string &name, float x, float y, float z) {
  auto loc = glGetUniformLocation(programID, name.c_str());
  if (loc == -1) {
    throw "uniform not found";
  }
  glProgramUniform3f(programID, loc, x, y, z);
}

void Shader::SetUniformMatrix(const std::string &name, const Matrix4 &value) {
  auto loc = glGetUniformLocation(programID, name.c_str());
  if (loc == -1) {
    throw "uniform not found";
  }
  glProgramUniformMatrix4fv(programID, loc, 1, true, &value.values[0]);
}
}