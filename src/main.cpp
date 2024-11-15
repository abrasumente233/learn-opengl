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
void mouse_pos_callback(GLFWwindow *, double xpos, double ypos);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float fov = 45.0f;

float last_frame_time = 0.0f;  // Time of last frame
float frame_delta_time = 0.0f; // Time between current frame and last frame

glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
float yaw = -90.0f, pitch = 0.0f;

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

unsigned int load_shader(const char *vs_path, const char *fs_path) {
  auto vs_src = read_file_to_string(vs_path);
  auto fs_src = read_file_to_string(fs_path);
  unsigned int vertex_shader =
    compile_shader(vs_src.value().c_str(), ShaderType::Vertex);
  unsigned int fragment_shader =
    compile_shader(fs_src.value().c_str(), ShaderType::Fragment);

  unsigned int shader_program = link_shaders(vertex_shader, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

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

  unsigned int obj_shader = load_shader("src/basic.vert", "src/basic.frag");
  unsigned int light_shader = load_shader("src/basic.vert", "src/light.frag");

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

  // set up VBO
  unsigned int VBO;
  {
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // set up object VAO
  unsigned int obj_vao;
  {
    // setup a vertex array object to store vertex attribute configurations.
    // vertex "array" basically means
    //
    //   1) vertex buffer object(s) (VBO) that stores vertex data.
    //   2) vertex attribute pointer(s) that specify how to interpret the data.
    //
    glGenVertexArrays(1, &obj_vao);
    glBindVertexArray(obj_vao);

    // link vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(/* location */ 0, /* size */ 3, GL_FLOAT, GL_FALSE,
                          /* stride */ va_stride, /* offset */ (void *)0);
    glEnableVertexAttribArray(0);

    // cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  unsigned int light_vao;
  {
    glGenVertexArrays(1, &light_vao);
    glBindVertexArray(light_vao);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(/* location */ 0, /* size */ 3, GL_FLOAT, GL_FALSE,
                          /* stride */ va_stride, /* offset */ (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // enable depth test
  glEnable(GL_DEPTH_TEST);

  // mouse input
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_pos_callback);

  while (!glfwWindowShouldClose(window)) {
    float current_frame_time = (float)glfwGetTime();
    frame_delta_time = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;

    process_input(window);

    // render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // camera coordinate system
    glm::vec3 camera_direction =
      glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch)));

    glm::vec3 camera_target = camera_pos + camera_direction;

    // building coordinate system of the camera
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camera_z = -camera_direction;
    glm::vec3 camera_x = glm::normalize(glm::cross(up, camera_z));
    glm::vec3 camera_y = glm::normalize(glm::cross(camera_z, camera_x));

    glm::mat4 view = glm::lookAt(camera_pos, camera_target, camera_y);

    glm::mat4 projection = glm::perspective(
      glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

    {
      glUseProgram(obj_shader);
      glm::vec3 pos = glm::vec3(0.0f, 0.0f, -3.0f);
      glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
      glUniformMatrix4fv(glGetUniformLocation(obj_shader, "model"), 1, GL_FALSE,
                         glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(obj_shader, "view"), 1, GL_FALSE,
                         glm::value_ptr(view));
      glUniformMatrix4fv(glGetUniformLocation(obj_shader, "projection"), 1,
                         GL_FALSE, glm::value_ptr(projection));

      glUniform3f(glGetUniformLocation(obj_shader, "objectColor"), 1.0f, 0.5f,
                  0.31f);
      glUniform3f(glGetUniformLocation(obj_shader, "lightColor"), 1.0f, 1.0f,
                  1.0f);

      glBindVertexArray(obj_vao);
      glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }

    {
      glUseProgram(light_shader);
      glm::vec3 pos = glm::vec3(1.2f, 1.0f, 2.0f);
      glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
      model = glm::scale(model, glm::vec3(0.2f));
      glUniformMatrix4fv(glGetUniformLocation(obj_shader, "model"), 1, GL_FALSE,
                         glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(obj_shader, "view"), 1, GL_FALSE,
                         glm::value_ptr(view));
      glUniformMatrix4fv(glGetUniformLocation(obj_shader, "projection"), 1,
                         GL_FALSE, glm::value_ptr(projection));

      glBindVertexArray(light_vao);
      glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }

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

void mouse_pos_callback(GLFWwindow *, double xpos, double ypos) {
  static double last_xpos = xpos, last_ypos = ypos;
  static bool first_call = true;

  if (first_call) {
    last_xpos = xpos;
    last_ypos = ypos;
    first_call = false;
  }

  double xoffset = xpos - last_xpos;
  double yoffset = last_ypos - ypos; // reversed since y-coordinates go from
                                     // bottom to top

  last_xpos = xpos;
  last_ypos = ypos;

  float sensitivity = 0.05f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f) {
    pitch = 89.0f;
  }
  if (pitch < -89.0f) {
    pitch = -89.0f;
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina
  // displays.
  glViewport(0, 0, width, height);
}
