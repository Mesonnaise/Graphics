#include<stdexcept>
#include "DescriptorPool.h"

DescriptorPool::DescriptorPool(DevicePtr device):mDevice(device){
  VkDescriptorPoolSize poolSizes[]={
    {
      .type=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount=1000
    },
    {
      .type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount=1000
    },
    {
      .type=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount=1000
    }
  };
  VkDescriptorPoolCreateInfo poolInfo={
    .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext=nullptr,
    .flags=VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    .maxSets=1000,
    .poolSizeCount=(uint32_t)(sizeof(poolSizes)/sizeof(poolSizes[0])),
    .pPoolSizes=poolSizes
  };
  auto result=vkCreateDescriptorPool(mDevice->Handle(),&poolInfo,nullptr,&mHandle);
  if(result!=VK_SUCCESS)
    throw std::runtime_error("Unable to create descriptor pool");
}

DescriptorPool::~DescriptorPool(){
  vkDestroyDescriptorPool(mDevice->Handle(),mHandle,nullptr);
}

std::vector<DescriptorPool::Descriptor> DescriptorPool::AllocateDescriptorSets(
  const std::vector<DescriptorSetLayoutPtr> &layouts){
  
  std::vector<VkDescriptorSetLayout> vkLayouts;
  std::vector<DescriptorPool::Descriptor> descriptors;
  for(auto &layout:layouts)
    vkLayouts.push_back(layout->Handle());
  VkDescriptorSetAllocateInfo allocInfo={
    .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext=nullptr,
    .descriptorPool=mHandle,
    .descriptorSetCount=(uint32_t)vkLayouts.size(),
    .pSetLayouts=vkLayouts.data()
  };
  std::vector<VkDescriptorSet> descriptorSetsHandles(vkLayouts.size());
  if(vkAllocateDescriptorSets(mDevice->Handle(),&allocInfo,descriptorSetsHandles.data())!=VK_SUCCESS)
    throw std::runtime_error("Unable to allocate descriptor sets");

  for(auto handle:descriptorSetsHandles)
    descriptors.emplace_back(Descriptor(mDevice,handle));
  
  return descriptors;
}

void DescriptorPool::Descriptor::WriteDescriptorSet(std::vector<BufferPtr> buffers,VkDescriptorType type){
  std::vector<VkDescriptorBufferInfo> bufferInfos(buffers.size(),{});
  std::vector<VkWriteDescriptorSet> descriptorSets(buffers.size(),{});

  for(size_t i=0;i<buffers.size();i++){
    VkDescriptorBufferInfo bufferInfo={
      .buffer=buffers[i]->Handle(),
      .offset=0,
      .range=VK_WHOLE_SIZE
    };
    bufferInfos[i]=bufferInfo;
    VkWriteDescriptorSet writeInfo={
      .sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext=nullptr,
      .dstSet=mHandle,
      .dstBinding=(uint32_t)i,
      .dstArrayElement=0,
      .descriptorCount=1,
      .descriptorType=type,
      .pImageInfo=nullptr,
      .pBufferInfo=&bufferInfos[i],
      .pTexelBufferView=nullptr
    };
    descriptorSets[i]=writeInfo;
  }

  vkUpdateDescriptorSets(mDevice->Handle(),(uint32_t)buffers.size(),descriptorSets.data(),0,nullptr);
}

void DescriptorPool::BindDescriptorSet(VkCommandBuffer cmdBuffer,
  PipelineLayoutPtr pipelineLayout,
  uint32_t setIndex,
  std::vector<Descriptor> descriptor){
  
  std::vector<VkDescriptorSet> vkDescriptorSets;
  for(auto &desc:descriptor)
    vkDescriptorSets.push_back(desc.mHandle);
  
  vkCmdBindDescriptorSets(
    cmdBuffer,
    pipelineLayout->PipelineBindPoint(),
    pipelineLayout->Handle(),
    setIndex,
    (uint32_t)vkDescriptorSets.size(),
    vkDescriptorSets.data(),
    0,
    nullptr);
}