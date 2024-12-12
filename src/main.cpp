#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
// This line is necessary. glad must be included before glfw.
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "texture.hpp"

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 800;
const float ASPECT_RATIO = (float)SCR_WIDTH / SCR_HEIGHT;

float last_frame_time = 0.0f;  // Time of last frame
float frame_delta_time = 0.0f; // Time between current frame and last frame

Camera camera(glm::vec3(-0.48f, -0.49f, 0.57f), -49.65f, 37.75f);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_pos_callback(GLFWwindow *, double xpos, double ypos);
void cursor_enter_callback(GLFWwindow *window, int entered);
void process_input(GLFWwindow *window);

bool camera_active = true;
void set_camera_active(bool active) {
  camera_active = active;
  ImGuiIO &io = ImGui::GetIO();
  if (active) {
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);
  } else {
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

void render_imgui_window(
  const Camera &camera, bool &spotlight_enabled, float &spotlight_cutoff,
  float &spotlight_outer_cutoff, glm::vec3 &spotlight_ambient,
  glm::vec3 &spotlight_diffuse, glm::vec3 &spotlight_specular,
  glm::vec3 &directional_dir, glm::vec3 &directional_ambient,
  glm::vec3 &directional_diffuse, glm::vec3 &directional_specular,
  std::array<glm::vec3, 4> &point_light_positions,
  std::array<glm::vec3, 4> &point_light_colors, float &point_light_constant,
  float &point_light_linear, float &point_light_quadratic) {

  ImGui::Begin("Scene Controls");

  if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Camera Controls:");
    ImGui::BulletText("WASD - Move camera");
    ImGui::BulletText("Mouse - Look around");
    ImGui::BulletText("ESC - Toggle camera/cursor");
    ImGui::BulletText("Cmd+W - Close window");
  }

  if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.position.x,
                camera.position.y, camera.position.z);
    ImGui::Text("Camera Yaw: %.2f, Pitch: %.2f", camera.yaw, camera.pitch);
  }

  if (ImGui::CollapsingHeader("Directional Light",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::DragFloat3("Direction##Dir", glm::value_ptr(directional_dir), 0.1f);
    ImGui::ColorEdit3("Ambient##Dir", glm::value_ptr(directional_ambient));
    ImGui::ColorEdit3("Diffuse##Dir", glm::value_ptr(directional_diffuse));
    ImGui::ColorEdit3("Specular##Dir", glm::value_ptr(directional_specular));
  }

  if (ImGui::CollapsingHeader("Spotlight", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox("Enabled##Spot", &spotlight_enabled);
    ImGui::ColorEdit3("Ambient##Spot", glm::value_ptr(spotlight_ambient));
    ImGui::ColorEdit3("Diffuse##Spot", glm::value_ptr(spotlight_diffuse));
    ImGui::ColorEdit3("Specular##Spot", glm::value_ptr(spotlight_specular));
    ImGui::SliderFloat("Cutoff Angle##Spot", &spotlight_cutoff, 0.0f, 90.0f);
    ImGui::SliderFloat("Outer Cutoff Angle##Spot", &spotlight_outer_cutoff,
                       0.0f, 90.0f);

    // Keep outer cutoff always larger than inner cutoff
    if (spotlight_outer_cutoff < spotlight_cutoff)
      spotlight_outer_cutoff = spotlight_cutoff;
  }
  if (ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Attenuation settings
    ImGui::Text("Attenuation:");
    ImGui::SliderFloat("Constant", &point_light_constant, 0.0f, 1.0f);
    ImGui::SliderFloat("Linear", &point_light_linear, 0.0f, 1.0f);
    ImGui::SliderFloat("Quadratic", &point_light_quadratic, 0.0f, 1.0f);

    // Individual point light controls
    for (int i = 0; i < 4; i++) {
      if (ImGui::TreeNode(("Point Light " + std::to_string(i + 1)).c_str())) {
        ImGui::DragFloat3("Position", glm::value_ptr(point_light_positions[i]),
                          0.1f);
        ImGui::ColorEdit3("Color", glm::value_ptr(point_light_colors[i]));
        ImGui::TreePop();
      }
    }
  }

  ImGui::End();
}

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
    set_camera_active(camera_active);
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
    glVertexAttribPointer(/* location */ 2, /* size */ 2, GL_FLOAT, GL_FALSE,
                          /* stride */ va_stride,
                          /* offset */ (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

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

  Texture lamp_tex("./assets/redstone-lamp.png");
  Texture container_tex("./assets/container2.png");
  Texture container_specular_tex("./assets/container2-specular-map.png");

  // draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // enable depth test
  glEnable(GL_DEPTH_TEST);

  // mouse input
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_pos_callback);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);

  // lighting rotation settings
  // float radius = 4.0f;
  // glm::vec3 rotation_center = glm::vec3(0.0f, 0.0f, 3.0f);
  // glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);

  // light color
  glm::vec3 spotlight_ambient(0.1f, 0.1f, 0.1f);
  glm::vec3 spotlight_diffuse(1.0f, 1.0f, 1.0f);
  glm::vec3 spotlight_specular(1.0f, 1.0f, 1.0f);

  glm::vec3 directional_dir(0.0f, -1.0f, -1.0f);
  glm::vec3 directional_ambient(0.05f);
  glm::vec3 directional_diffuse(0.4f);
  glm::vec3 directional_specular(0.5f);

  std::array<glm::vec3, 4> point_light_positions = {
    glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(1.1f, 0.3f, 0.3f)};

  std::array<glm::vec3, 4> point_light_colors = {
    glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f)};
  float point_light_constant = 1.0f;
  float point_light_linear = 0.09f;
  float point_light_quadratic = 0.032f;

  bool spotlight_enabled = false;
  float spotlight_cutoff = 12.5f;
  float spotlight_outer_cutoff = 20.5f;

  Model backpack_model("./assets/backpack/backpack.obj");
  Model sponza_model("./assets/sponza/sponza.obj");

  while (!glfwWindowShouldClose(window)) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    render_imgui_window(
      camera, spotlight_enabled, spotlight_cutoff, spotlight_outer_cutoff,
      spotlight_ambient, spotlight_diffuse, spotlight_specular, directional_dir,
      directional_ambient, directional_diffuse, directional_specular,
      point_light_positions, point_light_colors, point_light_constant,
      point_light_linear, point_light_quadratic);

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

    // glm::vec3 rotation_point;
    // if (abs(rotation_axis.y) < abs(rotation_axis.x)) {
    //   rotation_point =
    //     radius *
    //     glm::normalize(glm::cross(rotation_axis, glm::vec3(0.0f, 1.0f,
    //     0.0f)));
    // } else {
    //   rotation_point =
    //     radius *
    //     glm::normalize(glm::cross(rotation_axis, glm::vec3(1.0f, 0.0f,
    //     0.0f)));
    // }
    //
    // glm::mat4 light_model = glm::mat4(1.0f);
    // light_model = glm::translate(light_model, rotation_center);
    // light_model = glm::rotate(light_model, time, rotation_axis);
    // light_model = glm::translate(light_model, rotation_point);
    // light_model = glm::scale(light_model, glm::vec3(0.5f));
    //
    // glm::vec3 light_world =
    //   glm::vec3(light_model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    // glm::vec3 light_view = glm::vec3(view * glm::vec4(light_world, 1.0f));

    glm::mat4 normal_matrix = glm::transpose(glm::inverse(view));

    {
      obj_shader.use();
      obj_shader.set_mat4("view", view);
      obj_shader.set_mat4("projection", projection);
      obj_shader.set_mat4("normalMatrix", normal_matrix);
      obj_shader.set_vec3("spotlight.pos", glm::vec3(0.0f));
      obj_shader.set_vec3("spotlight.dir", glm::vec3(0.0f, 0.0f, -1.0f));
      obj_shader.set_float("spotlight.cutoff",
                           glm::cos(glm::radians(spotlight_cutoff)));
      obj_shader.set_float("spotlight.outerCutoff",
                           glm::cos(glm::radians(spotlight_outer_cutoff)));

      if (spotlight_enabled) {
        obj_shader.set_vec3("spotlight.ambient", spotlight_ambient);
        obj_shader.set_vec3("spotlight.diffuse", spotlight_diffuse);
        obj_shader.set_vec3("spotlight.specular", spotlight_specular);
      } else {
        obj_shader.set_vec3("spotlight.ambient", glm::vec3(0.0f));
        obj_shader.set_vec3("spotlight.diffuse", glm::vec3(0.0f));
        obj_shader.set_vec3("spotlight.specular", glm::vec3(0.0f));
      }

      obj_shader.set_vec3("directionalLight.dir", directional_dir);
      obj_shader.set_vec3("directionalLight.ambient", directional_ambient);
      obj_shader.set_vec3("directionalLight.diffuse", directional_diffuse);
      obj_shader.set_vec3("directionalLight.specular", directional_specular);

      for (size_t i = 0; i < 4; i++) {
        std::string name = "pointLights[" + std::to_string(i) + "]";
        glm::vec3 viewPos =
          glm::vec3(view * glm::vec4(point_light_positions[i], 1.0f));
        obj_shader.set_vec3(name + ".pos", viewPos);
        obj_shader.set_float(name + ".constant", point_light_constant);
        obj_shader.set_float(name + ".linear", point_light_linear);
        obj_shader.set_float(name + ".quadratic", point_light_quadratic);
        obj_shader.set_vec3(name + ".ambient", point_light_colors[i] * 0.05f);
        obj_shader.set_vec3(name + ".diffuse", point_light_colors[i] * 0.8f);
        obj_shader.set_vec3(name + ".specular", point_light_colors[i]);
      }

      // obj_shader.set_texture("material.diffuse", container_tex, 0);
      // obj_shader.set_texture("material.specular", container_specular_tex, 1);
      obj_shader.set_float("material.shininess", 32.0f);

      // array of cubes
      // const size_t ncubes = 25;
      // const size_t cubes_per_row = 5;
      // const float spacing = 1.2f;
      // glBindVertexArray(obj_vao);
      // for (size_t i = 0; i < ncubes; i++) {
      //   const float off = (cubes_per_row - 1) * spacing / 2.0f;
      //   const float x = (i % cubes_per_row) * spacing - off;
      //   const float y = (i / cubes_per_row) * spacing - off;
      //
      //   glm::vec3 pos = glm::vec3(x, y, 0.0f);
      //   glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
      //   obj_shader.set_mat4("model", model);
      //   glDrawArrays(GL_TRIANGLES, 0, num_vertices);
      // }

      // backpack
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
      model =
        glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::scale(model, glm::vec3(0.2f));
      obj_shader.set_mat4("model", model);
      backpack_model.draw(obj_shader);

      // sponza
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f));
      model = glm::scale(model, glm::vec3(0.01f));
      obj_shader.set_mat4("model", model);
      sponza_model.draw(obj_shader);
    }

#if 1
    for (size_t i = 0; i < point_light_positions.size(); i++) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, point_light_positions[i]);
      model = glm::scale(model, glm::vec3(0.2f));

      light_shader.use();
      light_shader.set_mat4("model", model);
      light_shader.set_mat4("view", view);
      light_shader.set_mat4("projection", projection);

      light_shader.set_texture("lampTexture", lamp_tex, 0);

      glBindVertexArray(light_vao);
      glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
#endif

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
    static float last_press = 0.0f;
    float current_time = (float)glfwGetTime();

    // Debounce the ESC key
    if (current_time - last_press > 0.3f) {
      bool active = !camera_active;
      set_camera_active(active);
    }
    last_press = current_time;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS &&
      (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
       glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (camera_active) {
    camera.update_keyboard(window, frame_delta_time);
  }
}

static double last_xpos = 0.0, last_ypos = 0.0;
void mouse_pos_callback(GLFWwindow *, double xpos, double ypos) {
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

  if (camera_active) {
    camera.update_mouse(xoffset, yoffset);
  }
}

void cursor_enter_callback(GLFWwindow *window, int entered) {
  if (entered) {
    // Reset mouse position when cursor re-enters window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    last_xpos = xpos;
    last_ypos = ypos;
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
