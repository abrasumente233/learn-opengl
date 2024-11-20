#pragma once

#include <tuple>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  glm::vec3 position;
  float yaw = -90.0f;
  float pitch = 0.0f;
  float fov = 45.0f;

  Camera(glm::vec3 position) : position(position) {}
  Camera(glm::vec3 position, float yaw, float pitch, float fov)
    : position(position), yaw(yaw), pitch(pitch), fov(fov) {}

  glm::mat4 view() const {
    auto [front, right, up] = calculate_basis();
    return glm::lookAt(position, position + front, up);
  }

  glm::mat4 projection(float aspect_ratio) const {
    return glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 100.0f);
  }

  glm::vec3 front() const {
    auto [front, right, up] = calculate_basis();
    return front;
  }

  void update_keyboard(GLFWwindow *window, float delta_time) {
    float velocity = 2.5f * delta_time;
    auto [front, right, up] = calculate_basis();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      position += front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      position -= front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      position -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      position += right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      position.y += velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      position.y -= velocity;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
      fov = glm::clamp(fov + 1.0f, 1.0f, 90.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
      fov = glm::clamp(fov - 1.0f, 1.0f, 90.0f);
  }

  void update_mouse(float xoffset, float yoffset, float sensitivity = 0.05f) {
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
  }

private:
  std::tuple<glm::vec3, glm::vec3, glm::vec3> calculate_basis() const {
    glm::vec3 front = {cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                       sin(glm::radians(pitch)),
                       sin(glm::radians(yaw)) * cos(glm::radians(pitch))};
    front = glm::normalize(front);

    glm::vec3 world_up = {0.0f, 1.0f, 0.0f};
    glm::vec3 right = glm::normalize(glm::cross(front, world_up));
    glm::vec3 up = glm::normalize(glm::cross(right, front));

    return {front, right, up};
  }
};
