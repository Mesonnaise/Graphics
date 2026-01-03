#pragma once
#include<memory>
#include<vector>
#include<vulkan/vulkan.h>
#include"Device.h"
class DescriptorSetLayout:public std::enable_shared_from_this<DescriptorSetLayout>{
public:
  using Bindings=std::vector<VkDescriptorSetLayoutBinding>;
private:
  PFN_vkGetDescriptorSetLayoutBindingOffsetEXT pfGetDescriptorSetLayoutBindingOffset=nullptr;

  DevicePtr mDevice=nullptr;
  VkDescriptorSetLayout mHandle=nullptr;
  
  Bindings mBinding;

protected:
  DescriptorSetLayout(DevicePtr device,Bindings bindings);
public:
  static inline auto Create(DevicePtr device,Bindings bindings){
    auto p=new DescriptorSetLayout(device,bindings);
    return std::shared_ptr<DescriptorSetLayout>(p);
  }

  constexpr VkDescriptorSetLayout Handle()const{
    return mHandle;
  }

  constexpr std::vector<VkDescriptorSetLayoutBinding> Binding(){
    return mBinding;
  }

  constexpr VkShaderStageFlags Stages(){
    VkShaderStageFlags flags=0;
    for(auto &binding:mBinding)
      flags|=binding.stageFlags;
    return flags;
  }

  ~DescriptorSetLayout();

  VkDeviceAddress BindingMemoryOffset(uint32_t BindingIndex);
  VkDeviceSize GetLayoutSize();
};

using DescriptorSetLayoutPtr=std::shared_ptr<DescriptorSetLayout>;
