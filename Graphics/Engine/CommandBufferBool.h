#pragma once
#include<memory>
#include<vector>
#include<vulkan/vulkan.h>
#include"Device.h"

class CommandBufferBool:public std::enable_shared_from_this<CommandBufferBool>{
  DevicePtr mDevice=nullptr;

  VkCommandPool mHandle=nullptr;
protected:
  CommandBufferBool(DevicePtr &device,uint32_t queueIndex);

public:
  static auto Create(DevicePtr &device,uint32_t queueIndex){
    auto p=new CommandBufferBool(device,queueIndex);
    return std::shared_ptr<CommandBufferBool>(p);
  }

  ~CommandBufferBool();

  std::vector<VkCommandBuffer> AllocateBuffers(uint32_t count);
  void Reset();
};

using CommandBufferBoolPtr=std::shared_ptr<CommandBufferBool>;