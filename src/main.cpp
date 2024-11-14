#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <glad/glad.h>
// This line is necessary. glad must be included before glfw.
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define LEARNGL_DEBUG

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float mix_value = 0.2f;
float fov = 45.0f;

float last_frame_time = 0.0f;  // Time of last frame
float frame_delta_time = 0.0f; // Time between current frame and last frame

glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);

std::optional<std::string> read_file_to_string(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

enum class ShaderType {
  Vertex,
  Fragment,
};

// NOTE! SHADER COMPILER.
//
// previously i claimed that compiler lives in the device driver layer, but
// now it seems incorrect. i think the shader compiler here actually translates
// the shader source into a format that... i don't know, into what? into some
// intermediate representation that are unified across OpenGL implementations,
// a.k.a vendor-independent or specified by OpenGL standard, or each vendor has
// to compile GLSL source by themselves into a format that's only recognizable
// by their own device driver?
//
// Oh, I see, it can't be vendor-independent. Otherwise game developers would
// only need to ship the unified intermediate representation, and the "compiling
// shaders" on the loading screen wouldn't take 20 minutes, because shaders are
// already shipped in an optimized format.
//
// So unfortunately, not only every device driver need to roll their own GLSL
// compiler, (if they don't use Mesa or some other solutions), they also pay the
// price of stupidly long shader compile time.
//
// Also having to roll a compiler by themselves means there are good and bad
// compilers.
//
// NIR basically solves the "unified intermediate representation" problem, but
// you still have the long compile time problem.
//
// UPDATE in 2024/05/19: oh, I was talking about SPIR-V.
// Reference: https://en.wikipedia.org/wiki/Vulkan#Pre-compiled_shaders
// So according to wikipedia, "A vulkan driver are supposed to ingest shaders
// already in SPIR-V format". So I guess all that is left is GPU-specific
// optimizations and code generation, which is a lot easier and deinfitely
// faster than compiling GLSL source. I wouldn't say it's instant, if you are
// still using LLVM for absolutely no reason? Oh it's the the fucking Liebig's
// Law all over again: you still have to support the apps that ship OpenGL &
// GLSL shaders, so of course you will still want that dragon to be here. And
// now that you have dragon at home, SPIR-V codegen is going to use The Dragon
// as well. Surely you don't want two separate codegen infrastructure. Of course
// Mesa now has a competing compiler infrastructure around NIR and you can use
// that. Also Mesa provides reusable bits for writing drivers, I wonder how LLVM
// people writes their drivers?
unsigned int compile_shader(const char *source, ShaderType shader_type) {
  unsigned int shader = glCreateShader(
    shader_type == ShaderType::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  // check if compile is successful
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {

#ifdef LEARNGL_DEBUG
    char info_log[512];
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
            info_log);
#endif
    return 0;
  }

  return shader;
}

// link vertex and fragment shader into a shader program.
// returns the program id
unsigned int link_shaders(unsigned int vshader, unsigned fshader) {
  unsigned int shader_program;
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vshader);
  glAttachShader(shader_program, fshader);
  glLinkProgram(shader_program);

  // check if link is successful
  int success;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {

#ifdef LEARNGL_DEBUG
    char info_log[512];
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", info_log);
#endif

    return 0;
  }

  return shader_program;
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
    glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return -1;
  }

  int nr_attributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nr_attributes);
  printf("Maximum nr of vertex attributes supported: %d\n", nr_attributes);

  auto vs_src = read_file_to_string("src/basic.vert");
  auto fs_src = read_file_to_string("src/basic.frag");

  unsigned int vs = compile_shader(vs_src.value().c_str(), ShaderType::Vertex);
  unsigned int fs =
    compile_shader(fs_src.value().c_str(), ShaderType::Fragment);

  unsigned int shader_program = link_shaders(vs, fs);

  // delete shader objects after linking.
  glDeleteShader(vs);
  glDeleteShader(fs);

  // prepare vertex data
  const float cx = 0.5f, cy = 0.5f;
  const float d = 0.5f;
  const float xl = cx - d, xr = cx + d;
  const float yb = cy - d, yt = cy + d;
  // clang-format off
  float vertices[] = {
    // positions         // texture coords
    // front
     0.5f, -0.5f,  0.5f, xr, yb, // fbr
    -0.5f, -0.5f,  0.5f, xl, yb, // fbl
    -0.5f,  0.5f,  0.5f, xl, yt, // ftl
     0.5f, -0.5f,  0.5f, xr, yb, // fbr
    -0.5f,  0.5f,  0.5f, xl, yt, // ftl
     0.5f,  0.5f,  0.5f, xr, yt, // ftr
    // right
     0.5f, -0.5f,  0.5f, xr, yb, // fbr
     0.5f,  0.5f,  0.5f, xr, yt, // ftr
     0.5f,  0.5f, -0.5f, xl, yt, // btr
     0.5f, -0.5f,  0.5f, xr, yb, // fbr
     0.5f,  0.5f, -0.5f, xl, yt, // btr
     0.5f, -0.5f, -0.5f, xl, yb, // bbr
    // bottom
     0.5f, -0.5f,  0.5f, xr, yb, // fbr
     0.5f, -0.5f, -0.5f, xl, yb, // bbr
    -0.5f, -0.5f, -0.5f, xl, yt, // bbl
     0.5f, -0.5f,  0.5f, xr, yb, // fbr
    -0.5f, -0.5f, -0.5f, xl, yt, // bbl
    -0.5f, -0.5f,  0.5f, xr, yt, // fbl
    // back
     0.5f,  0.5f, -0.5f, xr, yt, // btr
    -0.5f,  0.5f, -0.5f, xl, yt, // btl
    -0.5f, -0.5f, -0.5f, xl, yb, // bbl
     0.5f,  0.5f, -0.5f, xr, yt, // btr
    -0.5f, -0.5f, -0.5f, xl, yb, // bbl
     0.5f, -0.5f, -0.5f, xr, yb, // bbr
    // left
    -0.5f,  0.5f,  0.5f, xr, yt, // ftl
    -0.5f, -0.5f,  0.5f, xr, yb, // fbl
    -0.5f, -0.5f, -0.5f, xl, yb, // bbl
    -0.5f,  0.5f,  0.5f, xr, yt, // ftl
    -0.5f, -0.5f, -0.5f, xl, yb, // bbl
    -0.5f,  0.5f, -0.5f, xl, yt, // btl
    // top
     0.5f,  0.5f,  0.5f, xr, yb, // ftr
    -0.5f,  0.5f,  0.5f, xl, yb, // ftl
    -0.5f,  0.5f, -0.5f, xl, yt, // btl
     0.5f,  0.5f,  0.5f, xr, yb, // ftr
    -0.5f,  0.5f, -0.5f, xl, yt, // btl
     0.5f,  0.5f, -0.5f, xr, yt  // btr
  };
  // clang-format on
  size_t attr_stride = 5 * sizeof(float);
  size_t num_vertices = sizeof(vertices) / attr_stride;
  int va_stride = sizeof(vertices) / num_vertices;

  // ====== <VAO setup> ======

  // setup a vertex array object to store vertex attribute configurations.
  // vertex "array" basically means
  //
  //   1) vertex buffer object(s) (VBO) that stores vertex data.
  //   2) vertex attribute pointer(s) that specify how to interpret the data.
  //
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // setup vertex buffer object.
  unsigned int VBO;
  glGenBuffers(1, &VBO);

  // buffer type of a vertex buffer object is GL_ARRAY_BUFFER.
  // from now on any buffer calls we make (on the GL_ARRAY_BUFFER target) will
  // be used to configure the currently bound buffer, which is VBO.
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // copies vertex data into buffer's memory.
  //
  //   1) GL_STATIC_DRAW:  the data will most likely not change at all
  //                       or very rarely.
  //   2) GL_DYNAMIC_DRAW: the data is likely to change a lot.
  //   3) GL_STREAM_DRAW:  the data will change every time it is drawn.
  //
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // link vertex attributes
  // 0 is the location of the vertex attribute in the vertex shader.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, va_stride, (void *)0);
  // point the currently bound GL_ARRAY_BUFFER to location 0;
  glEnableVertexAttribArray(0);
  // in summary, `glVertexAttribPointer` declares the type
  // and `glEnableVertexAttribArray` tells the VBO the pointer.
  // i guess.
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, va_stride,
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // I've never seen API this ugly before.

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  // ====== <VAO setup /> ======

  // ====== <Texture> ======
  stbi_set_flip_vertically_on_load(true);
  unsigned int texture0, texture1;
  {
    int width, height, n_channels;
    unsigned char *texdata =
      stbi_load("./assets/container.jpg", &width, &height, &n_channels, 0);
    // stbi_load("./assets/awesomeface.png", &width, &height, &n_channels, 0);
    if (!texdata) {
      fprintf(stderr, "Failed to load texture\n");
      return -1;
    }

    glGenTextures(1, &texture0);
    glBindTexture(GL_TEXTURE_2D, texture0);

    auto mode = n_channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode,
                 GL_UNSIGNED_BYTE, texdata);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(texdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set border color
    // float border_color[] = {1.0f, 0.0f, 0.0f, 1.0f};
    // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
  }

  {
    int width, height, n_channels;
    unsigned char *texdata =
      stbi_load("./assets/awesomeface.png", &width, &height, &n_channels, 0);
    if (!texdata) {
      fprintf(stderr, "Failed to load texture\n");
      return -1;
    }
    // assert(n_channels == 4 && "awesomeface.png must have 4 channels");

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    auto mode = n_channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode,
                 GL_UNSIGNED_BYTE, texdata);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(texdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  // ====== <Texture /> ======

  // draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // since we only have one shader we can activate the shader once here.
  glUseProgram(shader_program);

  // for the same reason above, we can set the texture unit once here.
  glUniform1i(glGetUniformLocation(shader_program, "texture0"), 0);
  glUniform1i(glGetUniformLocation(shader_program, "texture1"), 1);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture1);

  // enable depth test
  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(window)) {
    float current_frame_time = (float)glfwGetTime();
    frame_delta_time = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;

    process_input(window);

    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float time = (float)glfwGetTime();
    float t = (sin(4.0f * time) / 2.0f) + 0.5f;
    int t_location = glGetUniformLocation(shader_program, "t");
    glUniform1f(t_location, t);
    glUniform1f(glGetUniformLocation(shader_program, "mixValue"), mix_value);

    // glm::mat4 model(1.0f);
    // model = glm::rotate(model, time * glm::radians(-55.0f),
    //                     glm::vec3(1.0f, 0.5f, 0.0f));
    // glm::rotate(model, glm::radians(-20.0f), glm::vec3(1.0f, 0.5f, 0.0f));

    // camera coordinate system
    glm::vec3 camera_target = camera_pos + glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camera_direction = glm::normalize(camera_pos - camera_target);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camera_right = glm::normalize(glm::cross(up, camera_direction));
    glm::vec3 camera_up =
      glm::normalize(glm::cross(camera_direction, camera_right));
    glm::mat4 view = glm::lookAt(camera_pos, camera_target, camera_up);

    glm::mat4 projection = glm::perspective(
      glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    // glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f,
    // 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1,
                       GL_FALSE, glm::value_ptr(projection));

    glm::vec3 cube_positions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

    glBindVertexArray(VAO);
    for (auto cube_position : cube_positions) {
      glm::mat4 model = glm::translate(glm::mat4(1.0f), cube_position);
      model = glm::rotate(model, (float)glfwGetTime() * glm::radians(20.0f),
                          cube_position);
      glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1,
                         GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, num_vertices);

      // glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int),
      //                GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

void knob(GLFWwindow *window, int key, float &value, float step, float min,
          float max) {
  if (glfwGetKey(window, key) == GLFW_PRESS) {
    value += step;
    if (value >= max) {
      value = max;
    }
    if (value <= min) {
      value = min;
    }
  }
}

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  knob(window, GLFW_KEY_UP, mix_value, 0.01f, 0.0f, 1.0f);
  knob(window, GLFW_KEY_DOWN, mix_value, -0.01f, 0.0f, 1.0f);

  const float camera_speed = 2.5f * frame_delta_time;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera_pos += camera_speed * glm::vec3(0.0f, 0.0f, -1.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera_pos += camera_speed * glm::vec3(0.0f, 0.0f, 1.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera_pos += camera_speed * glm::vec3(-1.0f, 0.0f, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera_pos += camera_speed * glm::vec3(1.0f, 0.0f, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
    camera_pos += camera_speed * glm::vec3(0.0f, -1.0f, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera_pos += camera_speed * glm::vec3(0.0f, 1.0f, 0.0f);
  }

  knob(window, GLFW_KEY_RIGHT, fov, 1.0f, 1.0f, 90.0f);
  knob(window, GLFW_KEY_LEFT, fov, -1.0f, 1.0f, 90.0f);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina
  // displays.
  glViewport(0, 0, width, height);
}
