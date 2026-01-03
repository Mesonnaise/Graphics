#pragma once
#include<memory>
#include<vector>
#include<vulkan/vulkan.h>

#include"Engine/Device.h"
#include"Engine/DescriptorSetLayout.h"
#include"Engine/MemoryHandler.h"
#include"Engine/PipelineLayout.h"
class DescriptorPool:public std::enable_shared_from_this<DescriptorPool>{
public: 
  class Descriptor{
    friend class DescriptorPool;
    DevicePtr mDevice=nullptr;
    VkDescriptorSet mHandle=nullptr;
  protected:
    Descriptor(DevicePtr device,VkDescriptorSet handle):mDevice(device),mHandle(handle){}
  public:
    void WriteDescriptorSet(std::vector<BufferPtr> buffers,VkDescriptorType type);
  };
  protected:
    DevicePtr mDevice=nullptr;
    VkDescriptorPool mHandle=nullptr;

protected:
  DescriptorPool(DevicePtr device);
public:
  static auto Create(DevicePtr device){
    auto p=new DescriptorPool(device);
    return std::shared_ptr<DescriptorPool>(p);
  }

  ~DescriptorPool();

  std::vector<DescriptorPool::Descriptor> AllocateDescriptorSets(
    const std::vector<DescriptorSetLayoutPtr> &layouts);

  void BindDescriptorSet(VkCommandBuffer cmdBuffer,
    PipelineLayoutPtr pipelineLayout,
    uint32_t setIndex,
    std::vector<Descriptor> descriptor);

};

using DescriptorPoolPtr=std::shared_ptr<DescriptorPool>;
