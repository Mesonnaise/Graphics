#include<set>
#include<algorithm>
#include<format>
#include<stdexcept>
#include "PipelineLayout.h"

static PFN_vkCmdSetDescriptorBufferOffsetsEXT pfnCmdSetDescriptorBufferOffsetsEXT=nullptr;
static PFN_vkCmdBindShadersEXT pfCmdBindShaders=nullptr;

static void LoadExtensionPointers(DevicePtr &device){
  if(pfnCmdSetDescriptorBufferOffsetsEXT==nullptr)
    pfCmdBindShaders=reinterpret_cast<PFN_vkCmdBindShadersEXT>(
      vkGetDeviceProcAddr(device->Handle(),"vkCmdBindShadersEXT"));
    pfnCmdSetDescriptorBufferOffsetsEXT=reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(
      vkGetDeviceProcAddr(device->Handle(),"vkCmdSetDescriptorBufferOffsetsEXT"));
}

PipelineLayout::PipelineLayout(DevicePtr &device,std::vector<ShaderObjectPtr> Shaders,std::vector<VkPushConstantRange> PushConstants):
  mDevice(device),mShaders(Shaders){
  LoadExtensionPointers(mDevice);

  std::vector<VkDescriptorSetLayout> layouts;
  bool firstShader=true;
  for(auto &shader:mShaders){
    if(firstShader){
      mBindPoint=shader->PipelineBindPoint();
      firstShader=false;
    } else if(mBindPoint!=shader->PipelineBindPoint())
      throw std::runtime_error(std::format("All shaders in a pipeline layout must have the same pipeline bind point:{} vs {}",
        mBindPoint==VK_PIPELINE_BIND_POINT_GRAPHICS?"Graphics":"Compute",
        shader->PipelineBindPoint()==VK_PIPELINE_BIND_POINT_GRAPHICS?"Graphics":"Compute"));

    auto shaderLayouts=shader->GetLayouts();
    for(auto &layout:shaderLayouts)
      layouts.push_back(layout->Handle());
  }

  /*VkPipelineDynamicStateCreateInfo dnymaic={
    .sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .pNext=nullptr,
    .flags=0,
    .dynamicStateCount=0,
    .pDynamicStates=nullptr
  };*/

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
    .sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext=nullptr,
    .flags=0,
    .setLayoutCount=(uint32_t)layouts.size(),
    .pSetLayouts=layouts.data(),
    .pushConstantRangeCount=(uint32_t)PushConstants.size(),
    .pPushConstantRanges=PushConstants.data(),
  };

  auto result=vkCreatePipelineLayout(mDevice->Handle(),&pipelineLayoutCreateInfo,nullptr,&mHandle);
  if(result!=VK_SUCCESS)
    throw std::runtime_error("Failed to create pipeline layout");
}

PipelineLayout::~PipelineLayout(){
  vkDestroyPipelineLayout(mDevice->Handle(),mHandle,nullptr);
}

VkDeviceSize PipelineLayout::GetDescriptorBufferTotalSize(){
  VkDeviceSize total=0;
  for(auto &shader:mShaders)
    total+=shader->GetDescriptorBufferTotalSize();

  return total;
}

void PipelineLayout::BindShaders(VkCommandBuffer CMDBuffer){
  std::set<VkShaderStageFlagBits> unusedStages={
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    VK_SHADER_STAGE_GEOMETRY_BIT,
    VK_SHADER_STAGE_FRAGMENT_BIT,
  };

  for(auto &shader:mShaders){
    auto currentShaderStages=shader->Stages();
    std::set<VkShaderStageFlagBits> shaderStageDiff;
    std::set_difference(unusedStages.begin(),unusedStages.end(),
      currentShaderStages.begin(),currentShaderStages.end(),
      std::inserter(shaderStageDiff,shaderStageDiff.begin()));
    unusedStages=shaderStageDiff;

    shader->BindShaders(CMDBuffer);
  }

  if(mBindPoint==VK_PIPELINE_BIND_POINT_GRAPHICS){
    std::vector<VkShaderEXT> unsedShaders(unusedStages.size(),nullptr);
    std::vector<VkShaderStageFlagBits> unusedStagesVec(unusedStages.begin(),unusedStages.end());
    pfCmdBindShaders(CMDBuffer,(uint32_t)unusedStagesVec.size(),unusedStagesVec.data(),unsedShaders.data());
  }
}

void PipelineLayout::SetDescriptorBufferOffsets(VkCommandBuffer CMDBuffer,std::vector<uint32_t> Indecies,std::vector<VkDeviceSize> Offsets){
  //pBufferIndices are which descriptorBuffers the layout set is assoscated with 
  //pOffsets are the offsets where the layout set is located in the descriptorBuffer

  uint32_t totalSetCount=0;
  for(auto &shader:mShaders)
    totalSetCount+=(uint32_t)shader->GetLayouts().size();
  
  pfnCmdSetDescriptorBufferOffsetsEXT(
    CMDBuffer,mBindPoint,
    mHandle,
    0,totalSetCount,
    Indecies.data(),Offsets.data());
}