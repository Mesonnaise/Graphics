#pragma once
#include<memory>
#include<vulkan/vulkan.h>

#include"Device.h"

namespace Engine{

  class DescriptorPool{
  private:


    DevicePtr mDevice=nullptr;
    VkDescriptorPool mHandle;
  protected:

    DescriptorPool(DevicePtr &device,uint32_t maxSets);
  public:
    static auto Create(DevicePtr &device,uint32_t maxSets){
      auto p=new DescriptorPool(device,maxSets);
      return std::shared_ptr<DescriptorPool>(p);
    }

    ~DescriptorPool();

    std::vector<VkDescriptorSet> Allocate(std::vector<VkDescriptorSetLayout> layouts);

  };

  using DescriptorPoolPtr=std::shared_ptr<DescriptorPool>;
}
