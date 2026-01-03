#pragma once
#include<array>
#include<vulkan/vulkan.h>

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>

namespace Engine{
  glm::mat4 CreateCameraViewMatrix(const glm::vec3 position,const glm::vec3 rotation);
  void BasicGraphicsPipeline(VkCommandBuffer CMDBuffer);
}