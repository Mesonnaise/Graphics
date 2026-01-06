#include"VulkanFunctions.h"

namespace Engine{
  extern PFN_vkGetDescriptorEXT pfnGetDescriptorEXT=nullptr;
  extern PFN_vkCmdBindDescriptorBuffersEXT pfnCmdBindDescriptorBuffersEXT=nullptr;
  extern PFN_vkGetDescriptorSetLayoutSizeEXT pfGetDescriptorSetLayoutSize=nullptr;
  extern PFN_vkGetDescriptorSetLayoutBindingOffsetEXT pfGetDescriptorSetLayoutBindingOffsetEXT=nullptr;

  extern PFN_vkCmdBindShadersEXT pfCmdBindShaders=nullptr;
  extern PFN_vkCreateShadersEXT pfCreateShader=nullptr;
  extern PFN_vkDestroyShaderEXT pfDestroyShader=nullptr;

  extern PFN_vkCmdSetDescriptorBufferOffsetsEXT pfnCmdSetDescriptorBufferOffsetsEXT=nullptr;
  extern PFN_vkCmdSetVertexInputEXT pfnCmdSetVertexInputEXT=nullptr;
  extern PFN_vkCmdSetPolygonModeEXT pfnCmdSetPolygonModeEXT=nullptr;
  extern PFN_vkCmdSetRasterizationSamplesEXT pfnCmdSetRasterizationSamplesEXT=nullptr;
  extern PFN_vkCmdSetSampleMaskEXT pfnCmdSetSampleMaskEXT=nullptr;
  extern PFN_vkCmdSetDepthClampEnableEXT pfnCmdSetDepthClampEnableEXT=nullptr;
  extern PFN_vkCmdSetAlphaToCoverageEnableEXT pfnCmdSetAlphaToCoverageEnableEXT=nullptr;
  extern PFN_vkCmdSetLogicOpEnableEXT pfnCmdSetLogicOpEnableEXT=nullptr;
  extern PFN_vkCmdSetColorBlendEnableEXT pfnCmdSetColorBlendEnableEXT=nullptr;
  extern PFN_vkCmdSetColorWriteMaskEXT pfnCmdSetColorWriteMaskEXT=nullptr;
  extern PFN_vkCmdSetProvokingVertexModeEXT pfnCmdSetProvokingVertexModeEXT=nullptr;


  void LoadVulkanFunctions(VkDevice device){
    if(pfnCmdSetColorWriteMaskEXT==nullptr){
      pfnGetDescriptorEXT=reinterpret_cast<PFN_vkGetDescriptorEXT>(
        vkGetDeviceProcAddr(device,"vkGetDescriptorEXT"));
      pfnCmdBindDescriptorBuffersEXT=reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(
        vkGetDeviceProcAddr(device,"vkCmdBindDescriptorBuffersEXT"));
      pfGetDescriptorSetLayoutSize=reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(
        vkGetDeviceProcAddr(device,"vkGetDescriptorSetLayoutSizeEXT"));
      pfGetDescriptorSetLayoutBindingOffsetEXT=reinterpret_cast<PFN_vkGetDescriptorSetLayoutBindingOffsetEXT>(
        vkGetDeviceProcAddr(device,"vkGetDescriptorSetLayoutBindingOffsetEXT"));

      pfCmdBindShaders=reinterpret_cast<PFN_vkCmdBindShadersEXT>(
        vkGetDeviceProcAddr(device,"vkCmdBindShadersEXT"));
      pfCreateShader=reinterpret_cast<PFN_vkCreateShadersEXT>(
        vkGetDeviceProcAddr(device,"vkCreateShadersEXT"));
      pfDestroyShader=reinterpret_cast<PFN_vkDestroyShaderEXT>(
        vkGetDeviceProcAddr(device,"vkDestroyShaderEXT"));

      pfnCmdSetDescriptorBufferOffsetsEXT=reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(
        vkGetDeviceProcAddr(device,"vkCmdSetDescriptorBufferOffsetsEXT"));
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
      pfnCmdSetProvokingVertexModeEXT=reinterpret_cast<PFN_vkCmdSetProvokingVertexModeEXT>(
        vkGetDeviceProcAddr(device,"vkCmdSetProvokingVertexModeEXT"));
    }
  }
}