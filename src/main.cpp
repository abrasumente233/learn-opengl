#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <glad/glad.h>
// This line is necessary. glad must be included before glfw.
#include <GLFW/glfw3.h>

#define LEARNGL_DEBUG

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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
  // float vertices[] = {
  //   // positions        // colors
  //   0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // top right
  //   0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
  //   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
  //   -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f  // top left
  // };

  // a proper triangle
  size_t num_vertices = 3;
  float vertices[] = {
    // positions        // start color    // end color
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
    0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f  // top
  };
  int va_stride = sizeof(vertices) / num_vertices;

  // unsigned int indices[] = {
  //   0, 1, 3, // first triangle
  //   // 1, 2, 3, // second triangle
  // };
  unsigned int indices[] = {
    0, 1, 2, // first triangle
    // 1, 2, 3, // second triangle
  };

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

  // set up element buffer object.
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // link vertex attributes
  // 0 is the location of the vertex attribute in the vertex shader.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, va_stride, (void *)0);
  // point the currently bound GL_ARRAY_BUFFER to location 0;
  glEnableVertexAttribArray(0);
  // in summary, `glVertexAttribPointer` declares the type
  // and `glEnableVertexAttribArray` tells the VBO the pointer.
  // i guess.
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, va_stride,
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, va_stride,
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // I've never seen API this ugly before.

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  // ====== <VAO setup /> ======

  // draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  while (!glfwWindowShouldClose(window)) {
    process_input(window);

    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float time = (float)glfwGetTime();
    float t = (sin(4.0f*time) / 2.0f) + 0.5f;
    int t_location = glGetUniformLocation(shader_program, "t");
    glUniform1f(t_location, t);

    // activate the shader program before rendering.
    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int),
                   GL_UNSIGNED_INT, 0);
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

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}
