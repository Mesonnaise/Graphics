#include "FastComputePipeline.h"

namespace Engine{
  FastComputePipeline::FastComputePipeline(DevicePtr &device,AllocatorPtr &allocator,std::filesystem::path &ShaderPath):
    FastPipeline(device,allocator,{ShaderPath}){}

  void FastComputePipeline::PopulateCommandBuffer(VkCommandBuffer CMDBuffer,uint32_t x,uint32_t y,uint32_t z){
    UpdateDescriptors();


    mLayout->BindShaders(CMDBuffer);
    WriteDescriptors(CMDBuffer);

    if(mPushConstantSize){
      if(mPushConstantSize!=mPushConstantData.size())
        throw std::runtime_error(std::format("Push constant size {} bytes, instead of {} bytes",mPushConstantData.size(),mPushConstantSize));

      VkShaderStageFlags stageFlags=0;
      for(auto stage:mShaderObject->Stages())
        stageFlags|=stage;

      VkPushConstantsInfo pushInfo={
        .sType=VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
        .pNext=nullptr,
        .layout=mLayout->Handle(),
        .stageFlags=stageFlags,
        .offset=0,
        .size=(uint32_t)mPushConstantData.size(),
        .pValues=mPushConstantData.data()
      };

      vkCmdPushConstants2(CMDBuffer,&pushInfo);
    }
    vkCmdDispatch(CMDBuffer,x,y,z);
  }
}