#pragma once
#include<memory>
#include<vector>
#include<vulkan/vulkan.h>
#include<spirv_cross/spirv_glsl.hpp>
#include"Common.h"

class Reflection{
public:
  struct BindingSet{
    VkDescriptorSetLayoutBinding Binding;
    std::string Name;
    uint32_t SetIndex;
    uint64_t Hash;
  };

private:
  spirv_cross::Compiler mHandle;
  std::string mEntryName;
  VkShaderStageFlagBits mStage;

  std::vector<DescriptorLayoutBinding> mBindings;

protected:
  Reflection(std::vector<uint32_t> ByteCode);
  void ProcessResourceBuffers(spirv_cross::SmallVector<spirv_cross::Resource> &Resource,VkDescriptorType DescriptorType);
  void processResourceImages(spirv_cross::SmallVector<spirv_cross::Resource> &Resource,VkDescriptorType DescriptorType);
public:
  static auto Create(std::vector<uint32_t> ByteCode){
    auto p=new Reflection(ByteCode);
    return std::shared_ptr<Reflection>(p);
  }

  std::vector<DescriptorLayoutBinding> DescriptorBindings()const;

  std::string EntryName()const;
  VkShaderStageFlagBits Stage()const;
};

using ReflectionPtr=std::shared_ptr<Reflection>;