#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include "texture.hpp"

class Shader {
public:
  unsigned int id;

  Shader(const char *vertex_path, const char *fragment_path) {
    auto vs_src = read_file_to_string(vertex_path);
    auto fs_src = read_file_to_string(fragment_path);

    unsigned int vertex =
      compile_shader(vs_src.value().c_str(), GL_VERTEX_SHADER);
    unsigned int fragment =
      compile_shader(fs_src.value().c_str(), GL_FRAGMENT_SHADER);

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    check_link_errors(id);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }

  void use() const { glUseProgram(id); }

  void set_bool(const char *name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name), (int)value);
  }

  void set_int(const char *name, int value) const {
    glUniform1i(glGetUniformLocation(id, name), value);
  }

  void set_float(const char *name, float value) const {
    glUniform1f(glGetUniformLocation(id, name), value);
  }

  void set_vec3(const char *name, const glm::vec3 &value) const {
    glUniform3f(glGetUniformLocation(id, name), value.x, value.y, value.z);
  }

  void set_mat4(const char *name, const glm::mat4 &value) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE,
                       glm::value_ptr(value));
  }

  void set_texture(const char *name, const Texture &texture, int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    set_int(name, unit);
  }

private:
  std::optional<std::string> read_file_to_string(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  unsigned int compile_shader(const char *source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    check_compile_errors(shader, type);
    return shader;
  }

  void check_compile_errors(unsigned int shader, GLenum type) {
    int success;
    char info_log[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, info_log);
      std::string type_str = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
      fprintf(stderr, "ERROR::SHADER::%s::COMPILATION_FAILED\n%s\n",
              type_str.c_str(), info_log);
    }
  }

  void check_link_errors(unsigned int program) {
    int success;
    char info_log[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(program, 1024, NULL, info_log);
      fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", info_log);
    }
  }
};
