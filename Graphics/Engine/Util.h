#pragma once
#include<array>
#include<vulkan/vulkan.h>

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>


glm::mat4 CreateCameraViewMatrix(const glm::vec3 position,const glm::vec3 rotation);

extern PFN_vkCmdSetVertexInputEXT pfnCmdSetVertexInputEXT;
extern PFN_vkCmdSetPolygonModeEXT pfnCmdSetPolygonModeEXT;
extern PFN_vkCmdSetRasterizationSamplesEXT pfnCmdSetRasterizationSamplesEXT;
extern PFN_vkCmdSetSampleMaskEXT pfnCmdSetSampleMaskEXT;

extern PFN_vkCmdSetDepthClampEnableEXT pfnCmdSetDepthClampEnableEXT;
extern PFN_vkCmdSetAlphaToCoverageEnableEXT pfnCmdSetAlphaToCoverageEnableEXT;
extern PFN_vkCmdSetLogicOpEnableEXT pfnCmdSetLogicOpEnableEXT;
extern PFN_vkCmdSetColorBlendEnableEXT pfnCmdSetColorBlendEnableEXT;
extern PFN_vkCmdSetColorWriteMaskEXT pfnCmdSetColorWriteMaskEXT;

void LoadSymbols(VkDevice device);
void BasicGraphicsPipeline(VkCommandBuffer CMDBuffer);