#include<stdexcept>
#include "CommandBufferBool.h"

namespace Engine{
  CommandBufferBool::CommandBufferBool(DevicePtr &device,uint32_t queueIndex):mDevice(device){
    VkCommandPoolCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext=nullptr,
      .flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex=queueIndex
    };

    auto status=vkCreateCommandPool(mDevice->Handle(),&info,nullptr,&mHandle);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to create command buffer pool");
  }

  CommandBufferBool::~CommandBufferBool(){
    vkDestroyCommandPool(mDevice->Handle(),mHandle,nullptr);

  }

  std::vector<VkCommandBuffer> CommandBufferBool::AllocateBuffers(uint32_t count){
    std::vector<VkCommandBuffer> ret(count,nullptr);
    VkCommandBufferAllocateInfo info={
      .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext=nullptr,
      .commandPool=mHandle,
      .level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount=count
    };


    auto status=vkAllocateCommandBuffers(mDevice->Handle(),&info,ret.data());
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to allocate command buffers");

    return ret;
  }

  void CommandBufferBool::Reset(){
    //Reseting the command pool is causing a memory leak in the current Vulkan SDK
    vkResetCommandPool(mDevice->Handle(),mHandle,VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
  }
}