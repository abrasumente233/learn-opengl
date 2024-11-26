#pragma once

#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>

enum class TextureType {
  UNSPECIFIED, // dude
  DIFFUSE,
  SPECULAR,
};

class Texture {
public:
  unsigned int id;
  TextureType type;
  std::string path;

  Texture(const char *texture_path, TextureType type = TextureType::UNSPECIFIED)
    : type(type), path(texture_path) {
    int width, height, n_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data =
      stbi_load(texture_path, &width, &height, &n_channels, 0);
    if (!data) {
      fprintf(stderr, "Failed to load texture\n");
      stbi_image_free(data);
      return;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    auto mode = n_channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  }
};
