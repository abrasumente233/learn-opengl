#include <glad/glad.h>
// This line is necessary. glad must be included before glfw.
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "camera.hpp"
#include "shader.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_pos_callback(GLFWwindow *, double xpos, double ypos);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float ASPECT_RATIO = (float)SCR_WIDTH / SCR_HEIGHT;

float last_frame_time = 0.0f;  // Time of last frame
float frame_delta_time = 0.0f; // Time between current frame and last frame

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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

  Shader obj_shader = Shader("src/basic.vert", "src/basic.frag");
  Shader light_shader = Shader("src/basic.vert", "src/light.frag");

  // prepare vertex data
  const float cx = 0.5f, cy = 0.5f;
  const float d = 0.5f;
  const float xl = cx - d, xr = cx + d;
  const float yb = cy - d, yt = cy + d;
  // clang-format off
  float vertices[] = {
    // positions          // normals           // texture coords
    // front (normal = 0.0f, 0.0f, 1.0f)
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  xr, yb, // fbr
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  xl, yb, // fbl
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  xl, yt, // ftl
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  xr, yb, // fbr
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  xl, yt, // ftl
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  xr, yt, // ftr
    // right (normal = 1.0f, 0.0f, 0.0f)
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  xr, yb, // fbr
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  xr, yt, // ftr
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  xl, yt, // btr
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  xr, yb, // fbr
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  xl, yt, // btr
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  xl, yb, // bbr
    // bottom (normal = 0.0f, -1.0f, 0.0f)
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  xr, yb, // fbr
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  xl, yb, // bbr
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  xl, yt, // bbl
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  xr, yb, // fbr
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  xl, yt, // bbl
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  xr, yt, // fbl
    // back (normal = 0.0f, 0.0f, -1.0f)
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  xr, yt, // btr
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  xl, yt, // btl
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  xl, yb, // bbl
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  xr, yt, // btr
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  xl, yb, // bbl
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  xr, yb, // bbr
    // left (normal = -1.0f, 0.0f, 0.0f)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  xr, yt, // ftl
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  xr, yb, // fbl
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  xl, yb, // bbl
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  xr, yt, // ftl
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  xl, yb, // bbl
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  xl, yt, // btl
    // top (normal = 0.0f, 1.0f, 0.0f)
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  xr, yb, // ftr
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  xl, yb, // ftl
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  xl, yt, // btl
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  xr, yb, // ftr
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  xl, yt, // btl
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  xr, yt  // btr
  };
  // clang-format on
  size_t attr_stride = 8 * sizeof(float);
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
    glVertexAttribPointer(/* location */ 1, /* size */ 3, GL_FLOAT, GL_FALSE,
                          /* stride */ va_stride,
                          /* offset */ (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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
    float time = (float)glfwGetTime();
    frame_delta_time = time - last_frame_time;
    last_frame_time = time;

    process_input(window);

    // render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.view();
    glm::mat4 projection = camera.projection(ASPECT_RATIO);

    glm::mat4 light_model = glm::mat4(1.0f);
    light_model = glm::rotate(light_model, time, glm::vec3(0.0f, 1.0f, 1.0f));
    light_model = glm::translate(light_model, glm::vec3(0.0f, 1.5f, -1.0f));
    light_model = glm::scale(light_model, glm::vec3(0.2f));
    glm::vec3 light_world =
      glm::vec3(light_model * glm::vec4(0.0f, 0.0f, -0.0f, 1.0f));
    glm::vec3 light_view = glm::vec3(view * glm::vec4(light_world, 1.0f));
    glm::mat4 normal_matrix = glm::transpose(glm::inverse(view));

    {
      glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
      glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

      obj_shader.use();
      obj_shader.set_mat4("model", model);
      obj_shader.set_mat4("view", view);
      obj_shader.set_mat4("projection", projection);
      obj_shader.set_vec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
      obj_shader.set_vec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
      obj_shader.set_vec3("lightPos", light_view);
      obj_shader.set_mat4("normalMatrix", normal_matrix);

      glBindVertexArray(obj_vao);
      glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }

    {
      light_shader.use();
      light_shader.set_mat4("model", light_model);
      light_shader.set_mat4("view", view);
      light_shader.set_mat4("projection", projection);

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

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  camera.update_keyboard(window, frame_delta_time);
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
  // reversed since y-coordinates go from bottom to top
  double yoffset = last_ypos - ypos;

  last_xpos = xpos;
  last_ypos = ypos;

  camera.update_mouse(xoffset, yoffset);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina
  // displays.
  glViewport(0, 0, width, height);
}
