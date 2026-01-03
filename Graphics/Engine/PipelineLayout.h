#pragma once
#include<vector>
#include<memory>

#include<vulkan/vulkan.h>

#include"Device.h"
#include"ShaderObject.h"

class PipelineLayout:public std::enable_shared_from_this<PipelineLayout>{
  DevicePtr mDevice=nullptr;

  std::vector<ShaderObjectPtr> mShaders;
  VkPipelineLayout mHandle=nullptr;
  VkPipelineBindPoint mBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;

protected:
  PipelineLayout(DevicePtr &device,std::vector<ShaderObjectPtr> Shaders,std::vector<VkPushConstantRange> PushConstant={});

public:
  static auto Create(DevicePtr &device,std::vector<ShaderObjectPtr> Shaders,std::vector<VkPushConstantRange> PushConstants={}){
    auto p=new PipelineLayout(device,Shaders,PushConstants);
    return std::shared_ptr<PipelineLayout>(p);
  }

  ~PipelineLayout();
  constexpr VkPipelineLayout Handle()const{
    return mHandle;
  }
  constexpr VkPipelineBindPoint PipelineBindPoint()const{
    return mBindPoint;
  }


  VkDeviceSize GetDescriptorBufferTotalSize();

  void BindShaders(VkCommandBuffer CMDBuffer);
  void SetDescriptorBufferOffsets(VkCommandBuffer CMDBuffer,std::vector<uint32_t> Indecies,std::vector<VkDeviceSize> Offsets);
};

using PipelineLayoutPtr=std::shared_ptr<PipelineLayout>;

