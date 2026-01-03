#include<array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include"Util.h"

extern PFN_vkCmdSetVertexInputEXT pfnCmdSetVertexInputEXT=nullptr;
extern PFN_vkCmdSetPolygonModeEXT pfnCmdSetPolygonModeEXT=nullptr;
extern PFN_vkCmdSetRasterizationSamplesEXT pfnCmdSetRasterizationSamplesEXT=nullptr;
extern PFN_vkCmdSetSampleMaskEXT pfnCmdSetSampleMaskEXT=nullptr;

extern PFN_vkCmdSetDepthClampEnableEXT pfnCmdSetDepthClampEnableEXT=nullptr;
extern PFN_vkCmdSetAlphaToCoverageEnableEXT pfnCmdSetAlphaToCoverageEnableEXT=nullptr;
extern PFN_vkCmdSetLogicOpEnableEXT pfnCmdSetLogicOpEnableEXT=nullptr;
extern PFN_vkCmdSetColorBlendEnableEXT pfnCmdSetColorBlendEnableEXT=nullptr;
extern PFN_vkCmdSetColorWriteMaskEXT pfnCmdSetColorWriteMaskEXT=nullptr;




glm::mat4 CreateCameraViewMatrix(const glm::vec3 position,const glm::vec3 rotation){

  glm::vec3 rotRad=glm::radians(rotation);
  glm::quat q=glm::quat(rotRad);

  glm::vec3 forward=glm::vec3(0.0f,0.0f,-1.0f);
  glm::vec3 up=glm::vec3(0.0f,1.0f,0.0f);

  return glm::lookAt(position,position+forward,up);

}


void LoadSymbols(VkDevice device){
  pfnCmdSetVertexInputEXT=reinterpret_cast<PFN_vkCmdSetVertexInputEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetVertexInputEXT"));
  pfnCmdSetPolygonModeEXT=reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetPolygonModeEXT"));
  pfnCmdSetRasterizationSamplesEXT=reinterpret_cast<PFN_vkCmdSetRasterizationSamplesEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetRasterizationSamplesEXT"));
  pfnCmdSetSampleMaskEXT=reinterpret_cast<PFN_vkCmdSetSampleMaskEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetSampleMaskEXT"));
  pfnCmdSetDepthClampEnableEXT=reinterpret_cast<PFN_vkCmdSetDepthClampEnableEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetDepthClampEnableEXT"));
  pfnCmdSetAlphaToCoverageEnableEXT=reinterpret_cast<PFN_vkCmdSetAlphaToCoverageEnableEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetAlphaToCoverageEnableEXT"));
  pfnCmdSetLogicOpEnableEXT=reinterpret_cast<PFN_vkCmdSetLogicOpEnableEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetLogicOpEnableEXT"));
  pfnCmdSetColorBlendEnableEXT=reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetColorBlendEnableEXT"));
  pfnCmdSetColorWriteMaskEXT=reinterpret_cast<PFN_vkCmdSetColorWriteMaskEXT>(
    vkGetDeviceProcAddr(device,"vkCmdSetColorWriteMaskEXT"));
}


void BasicGraphicsPipeline(VkCommandBuffer CMDBuffer){
  VkSampleMask SampleMask=0x1;
  VkBool32 vkFalse=VK_FALSE;
  VkColorComponentFlags MaskColorComponent=
    VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|
    VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;

  vkCmdSetDepthTestEnable(CMDBuffer,VK_FALSE);
  vkCmdSetRasterizerDiscardEnable(CMDBuffer,VK_FALSE);
  vkCmdSetCullMode(CMDBuffer,VK_CULL_MODE_NONE);
  vkCmdSetStencilTestEnable(CMDBuffer,VK_FALSE);
  vkCmdSetDepthBiasEnable(CMDBuffer,VK_FALSE);
  vkCmdSetDepthBoundsTestEnable(CMDBuffer,VK_FALSE);
  pfnCmdSetPolygonModeEXT(CMDBuffer,VK_POLYGON_MODE_FILL);
  pfnCmdSetRasterizationSamplesEXT(CMDBuffer,VK_SAMPLE_COUNT_1_BIT);
  pfnCmdSetSampleMaskEXT(CMDBuffer,VK_SAMPLE_COUNT_1_BIT,&SampleMask);
  pfnCmdSetDepthClampEnableEXT(CMDBuffer,VK_FALSE);
  pfnCmdSetAlphaToCoverageEnableEXT(CMDBuffer,VK_FALSE);
  pfnCmdSetLogicOpEnableEXT(CMDBuffer,VK_FALSE);
  pfnCmdSetColorBlendEnableEXT(CMDBuffer,0,1,&vkFalse);
  pfnCmdSetColorWriteMaskEXT(CMDBuffer,0,1,&MaskColorComponent);
  vkCmdSetPrimitiveTopology(CMDBuffer,VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  vkCmdSetPrimitiveRestartEnable(CMDBuffer,VK_FALSE);
}