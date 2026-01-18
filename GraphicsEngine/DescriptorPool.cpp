#include<stdexcept>
#include<vector>
#include "DescriptorPool.h"


namespace Engine{
  DescriptorPool::DescriptorPool(DevicePtr &device,uint32_t maxSet):mDevice(device){
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,16});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,16});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,16});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,16});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,16});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,16});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,16});

    VkDescriptorPoolCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext=nullptr,
      .flags=0,
      .maxSets=maxSet,
      .poolSizeCount=(uint32_t)poolSizes.size(),
      .pPoolSizes=poolSizes.data()
    };


    auto status=vkCreateDescriptorPool(mDevice->Handle(),&info,nullptr,&mHandle);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to create descriptor pool");
  }

  DescriptorPool::~DescriptorPool(){
    vkDestroyDescriptorPool(mDevice->Handle(),mHandle,nullptr);
  }

  std::vector<VkDescriptorSet> DescriptorPool::Allocate(std::vector<VkDescriptorSetLayout> layouts){
    VkDescriptorSetAllocateInfo info={
      .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext=nullptr,
      .descriptorPool=mHandle,
      .descriptorSetCount=(uint32_t)layouts.size(),
      .pSetLayouts=layouts.data()
    };

    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.resize(layouts.size());
    auto result=vkAllocateDescriptorSets(mDevice->Handle(),&info,descriptorSets.data());
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to allocate descriptors");
    return descriptorSets;
  }

}