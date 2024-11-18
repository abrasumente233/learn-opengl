#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

struct Material {
  const char *name;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
};

// clang-format off
const Material materials[] = {
    {"emerald",      glm::vec3(0.0215f, 0.1745f, 0.0215f),     glm::vec3(0.07568f, 0.61424f, 0.07568f),    glm::vec3(0.633f, 0.727811f, 0.633f),      0.6f},
    {"jade",         glm::vec3(0.135f, 0.2225f, 0.1575f),      glm::vec3(0.54f, 0.89f, 0.63f),             glm::vec3(0.316228f, 0.316228f, 0.316228f), 0.1f},
    {"obsidian",     glm::vec3(0.05375f, 0.05f, 0.06625f),     glm::vec3(0.18275f, 0.17f, 0.22525f),       glm::vec3(0.332741f, 0.328634f, 0.346435f), 0.3f},
    {"pearl",        glm::vec3(0.25f, 0.20725f, 0.20725f),     glm::vec3(1.0f, 0.829f, 0.829f),            glm::vec3(0.296648f, 0.296648f, 0.296648f), 0.088f},
    {"ruby",         glm::vec3(0.1745f, 0.01175f, 0.01175f),   glm::vec3(0.61424f, 0.04136f, 0.04136f),    glm::vec3(0.727811f, 0.626959f, 0.626959f), 0.6f},
    {"turquoise",    glm::vec3(0.1f, 0.18725f, 0.1745f),       glm::vec3(0.396f, 0.74151f, 0.69102f),      glm::vec3(0.297254f, 0.30829f, 0.306678f),  0.1f},
    {"brass",        glm::vec3(0.329412f, 0.223529f, 0.027451f),glm::vec3(0.780392f, 0.568627f, 0.113725f), glm::vec3(0.992157f, 0.941176f, 0.807843f), 0.21794872f},
    {"bronze",       glm::vec3(0.2125f, 0.1275f, 0.054f),      glm::vec3(0.714f, 0.4284f, 0.18144f),       glm::vec3(0.393548f, 0.271906f, 0.166721f), 0.2f},
    {"chrome",       glm::vec3(0.25f, 0.25f, 0.25f),           glm::vec3(0.4f, 0.4f, 0.4f),                glm::vec3(0.774597f, 0.774597f, 0.774597f), 0.6f},
    {"copper",       glm::vec3(0.19125f, 0.0735f, 0.0225f),    glm::vec3(0.7038f, 0.27048f, 0.0828f),      glm::vec3(0.256777f, 0.137622f, 0.086014f), 0.1f},
    {"gold",         glm::vec3(0.24725f, 0.1995f, 0.0745f),    glm::vec3(0.75164f, 0.60648f, 0.22648f),    glm::vec3(0.628281f, 0.555802f, 0.366065f), 0.4f},
    {"silver",       glm::vec3(0.19225f, 0.19225f, 0.19225f),  glm::vec3(0.50754f, 0.50754f, 0.50754f),    glm::vec3(0.508273f, 0.508273f, 0.508273f), 0.4f},
    {"black plastic",glm::vec3(0.0f, 0.0f, 0.0f),              glm::vec3(0.01f, 0.01f, 0.01f),             glm::vec3(0.50f, 0.50f, 0.50f),             0.25f},
    {"cyan plastic", glm::vec3(0.0f, 0.1f, 0.06f),             glm::vec3(0.0f, 0.50980392f, 0.50980392f),  glm::vec3(0.50196078f, 0.50196078f, 0.50196078f), 0.25f},
    {"green plastic",glm::vec3(0.0f, 0.0f, 0.0f),              glm::vec3(0.1f, 0.35f, 0.1f),               glm::vec3(0.45f, 0.55f, 0.45f),             0.25f},
    {"red plastic",  glm::vec3(0.0f, 0.0f, 0.0f),              glm::vec3(0.5f, 0.0f, 0.0f),                glm::vec3(0.7f, 0.6f, 0.6f),                0.25f},
    {"white plastic",glm::vec3(0.0f, 0.0f, 0.0f),              glm::vec3(0.55f, 0.55f, 0.55f),             glm::vec3(0.70f, 0.70f, 0.70f),             0.25f},
    {"yellow plastic",glm::vec3(0.0f, 0.0f, 0.0f),             glm::vec3(0.5f, 0.5f, 0.0f),                glm::vec3(0.60f, 0.60f, 0.50f),             0.25f},
    {"black rubber", glm::vec3(0.02f, 0.02f, 0.02f),           glm::vec3(0.01f, 0.01f, 0.01f),             glm::vec3(0.4f, 0.4f, 0.4f),                0.078125f},
    {"cyan rubber",  glm::vec3(0.0f, 0.05f, 0.05f),            glm::vec3(0.4f, 0.5f, 0.5f),                glm::vec3(0.04f, 0.7f, 0.7f),               0.078125f},
    {"green rubber", glm::vec3(0.0f, 0.05f, 0.0f),             glm::vec3(0.4f, 0.5f, 0.4f),                glm::vec3(0.04f, 0.7f, 0.04f),              0.078125f},
    {"red rubber",   glm::vec3(0.05f, 0.0f, 0.0f),             glm::vec3(0.5f, 0.4f, 0.4f),                glm::vec3(0.7f, 0.04f, 0.04f),              0.078125f},
    {"white rubber", glm::vec3(0.05f, 0.05f, 0.05f),           glm::vec3(0.5f, 0.5f, 0.5f),                glm::vec3(0.7f, 0.7f, 0.7f),                0.078125f},
    {"yellow rubber",glm::vec3(0.05f, 0.05f, 0.0f),            glm::vec3(0.5f, 0.5f, 0.4f),                glm::vec3(0.7f, 0.7f, 0.04f),               0.078125f}
};
// clang-format on

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float ASPECT_RATIO = (float)SCR_WIDTH / SCR_HEIGHT;

float last_frame_time = 0.0f;  // Time of last frame
float frame_delta_time = 0.0f; // Time between current frame and last frame

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

int main() {
  glfwInit();

#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

  GLFWwindow *window =
    glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Setup ImGui
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
  }

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
    glVertexAttribPointer(/* location */ 2, /* size */ 2, GL_FLOAT, GL_FALSE,
                          /* stride */ va_stride,
                          /* offset */ (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  unsigned lamp_tex;
  {
    int width, height, n_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data =
      stbi_load("./assets/redstone-lamp.png", &width, &height, &n_channels, 0);
    if (!data) {
      fprintf(stderr, "Failed to load texture\n");
      return -1;
    }

    glGenTextures(1, &lamp_tex);
    glBindTexture(GL_TEXTURE_2D, lamp_tex);
    auto mode = n_channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
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
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    bool show_demo_window = true;
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Render();

    float time = (float)glfwGetTime();
    frame_delta_time = time - last_frame_time;
    last_frame_time = time;

    process_input(window);

    // render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.view();
    glm::mat4 projection = camera.projection(ASPECT_RATIO);

    float radius = 4.0f;
    glm::vec3 rotation_center = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 rotation_point;
    if (abs(rotation_axis.y) < abs(rotation_axis.x)) {
      rotation_point =
        radius *
        glm::normalize(glm::cross(rotation_axis, glm::vec3(0.0f, 1.0f, 0.0f)));
    } else {
      rotation_point =
        radius *
        glm::normalize(glm::cross(rotation_axis, glm::vec3(1.0f, 0.0f, 0.0f)));
    }

    glm::mat4 light_model = glm::mat4(1.0f);
    light_model = glm::translate(light_model, rotation_center);
    light_model = glm::rotate(light_model, time, rotation_axis);
    light_model = glm::translate(light_model, rotation_point);
    light_model = glm::scale(light_model, glm::vec3(0.5f));

    glm::vec3 light_world =
      glm::vec3(light_model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::vec3 light_view = glm::vec3(view * glm::vec4(light_world, 1.0f));
    glm::mat4 normal_matrix = glm::transpose(glm::inverse(view));

    {
      obj_shader.use();
      obj_shader.set_mat4("view", view);
      obj_shader.set_mat4("projection", projection);
      obj_shader.set_mat4("normalMatrix", normal_matrix);
      obj_shader.set_vec3("light.pos", light_view);
      glm::vec3 light(1.0f, 1.0f, 1.0f);
      obj_shader.set_vec3("light.ambient", light);
      obj_shader.set_vec3("light.diffuse", light);
      obj_shader.set_vec3("light.specular", light);

      const size_t nmaterials = sizeof(materials) / sizeof(Material);
      const size_t material_per_row = 5;
      const float spacing = 1.2f;
      glBindVertexArray(obj_vao);
      for (size_t i = 0; i < nmaterials; i++) {
        const float off = (material_per_row - 1) * spacing / 2.0f;
        const float x = (i % material_per_row) * spacing - off;
        const float y = (float)(i / material_per_row) * spacing - off;

        glm::vec3 pos = glm::vec3(x, y, 0.0f);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        obj_shader.set_mat4("model", model);
        obj_shader.set_vec3("material.ambient", materials[i].ambient);
        obj_shader.set_vec3("material.diffuse", materials[i].diffuse);
        obj_shader.set_vec3("material.specular", materials[i].specular);
        obj_shader.set_float("material.shininess",
                             materials[i].shininess * 128.0f);
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
      }
    }

    {
      light_shader.use();
      light_shader.set_mat4("model", light_model);
      light_shader.set_mat4("view", view);
      light_shader.set_mat4("projection", projection);
      light_shader.set_int("lampTexture", 0);

      glBindVertexArray(light_vao);
      glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse
    // moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // imgui cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS &&
      (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
       glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS)) {
    glfwSetWindowShouldClose(window, true);
  }

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
