#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

  const char *vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

  // NOTE! SHADER COMPILER.
  //
  // previously i claimed that compiler lives in the device driver layer, but
  // now it seems incorrect. i think the shader compiler here actually translates
  // the shader source into a format that... i don't know, into what? into some
  // intermediate representation that are unified across OpenGL implementations,
  // a.k.a vendor-independent or specified by OpenGL standard, or each vendor has to compile
  // GLSL source by themselves into a format that's only recognizable by their own
  // device driver?
  //
  // Oh, I see, it can't be vendor-independent. Otherwise game developers would
  // only need to ship the unified intermediate representation, and the "compiling
  // shaders" on the loading screen wouldn't take 20 minutes, because shaders are
  // already shipped in an optimized format.
  //
  // So unfortunately, not only every device driver need to roll their own GLSL compiler,
  // (if they don't use Mesa or some other solutions), they also pay the price of stupidly
  // long shader compile time.
  //
  // Also having to roll a compiler by themselves means there are good and bad compilers.
  //
  // NIR basically solves the "unified intermediate representation" problem, but you still
  // have the long compile time problem.
  //
  unsigned int vertex_shader;
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  // check if compile is successful
  int success;
  char info_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", info_log);
  }

  const char *fragment_shader_source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

  unsigned int fragment_shader;
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  // check if compile is successful
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", info_log);
  }

  // link vertex and fragment shader into a shader program.
  unsigned int shader_program;
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  // check if link is successful
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", info_log);
  }

  // delete shader objects after linking.
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // prepare vertex data
  float vertices[] = {
    0.5f,  0.5f, 0.0f, // top right
    0.5f, -0.5f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f  // top left
  };

  unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3, // second triangle
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
  //   GL_STATIC_DRAW:  the data will most likely not change at all or very rarely.
  //   GL_DYNAMIC_DRAW: the data is likely to change a lot.
  //   GL_STREAM_DRAW:  the data will change every time it is drawn.
  //
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // set up element buffer object.
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // link vertex attributes
  // 0 is the location of the vertex attribute in the vertex shader.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  // point the currently bound GL_ARRAY_BUFFER to location 0;
  glEnableVertexAttribArray(0);
  // in summary, `glVertexAttribPointer` declares the type
  // and `glEnableVertexAttribArray` tells the VBO the pointer.
  // i guess.

  // I've never seen API this ugly before.

  // ====== <VAO setup /> ======

  while (!glfwWindowShouldClose(window)) {
    process_input(window);

    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // activate the shader program before rendering.
    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    //glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}
