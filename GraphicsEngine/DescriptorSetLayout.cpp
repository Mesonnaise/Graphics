#include<stdexcept>
#include "DescriptorSetLayout.h"

#include"Internal/VulkanFunctions.h"

namespace Engine{
  DescriptorSetLayout::DescriptorSetLayout(DevicePtr device,Bindings bindings,bool useBuffer):mDevice(device),mBinding(bindings){
    VkDescriptorSetLayoutCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext=nullptr,
      .flags=0,
      .bindingCount=(uint32_t)bindings.size(),
      .pBindings=bindings.data()
    };

    if(useBuffer)
      info.flags=VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

    auto status=vkCreateDescriptorSetLayout(mDevice->Handle(),&info,nullptr,&mHandle);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to create descriptor set layout");
  }

  DescriptorSetLayout::~DescriptorSetLayout(){
    vkDestroyDescriptorSetLayout(mDevice->Handle(),mHandle,nullptr);
  }

  VkDeviceAddress DescriptorSetLayout::BindingMemoryOffset(uint32_t BindingIndex){
    VkDeviceAddress offset=0;
    pfGetDescriptorSetLayoutBindingOffset(
      mDevice->Handle(),
      mHandle,
      BindingIndex,
      &offset);

    return offset;

  }

  VkDeviceSize DescriptorSetLayout::GetLayoutSize(){
    VkDeviceSize layoutSize=0;
    pfGetDescriptorSetLayoutSize(mDevice->Handle(),mHandle,&layoutSize);
    return layoutSize;
  }
}