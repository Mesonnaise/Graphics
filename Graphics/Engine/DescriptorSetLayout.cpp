#include<stdexcept>
#include "DescriptorSetLayout.h"

static PFN_vkGetDescriptorSetLayoutSizeEXT pfGetDescriptorSetLayoutSize=nullptr;
static PFN_vkGetDescriptorSetLayoutBindingOffsetEXT pfGetDescriptorSetLayoutBindingOffset=nullptr;

static void loadExtensionPointers(DevicePtr &device){
  if(pfGetDescriptorSetLayoutSize==nullptr){
    pfGetDescriptorSetLayoutBindingOffset=
      reinterpret_cast<PFN_vkGetDescriptorSetLayoutBindingOffsetEXT>(
        vkGetDeviceProcAddr(device->Handle(),"vkGetDescriptorSetLayoutBindingOffsetEXT"));

    pfGetDescriptorSetLayoutSize=reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(
      vkGetDeviceProcAddr(device->Handle(),"vkGetDescriptorSetLayoutSizeEXT"));
  }
}

DescriptorSetLayout::DescriptorSetLayout(DevicePtr device,Bindings bindings):mDevice(device),mBinding(bindings){
  loadExtensionPointers(mDevice);

  VkDescriptorSetLayoutCreateInfo info={
    .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext=nullptr,
    .flags=VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
    .bindingCount=(uint32_t)bindings.size(),
    .pBindings=bindings.data()
  };

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