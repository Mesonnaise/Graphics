#pragma once
#include<cinttypes>
#include<string>
#include<map>
#include<vulkan/vulkan.h>

namespace Engine{
  struct DescriptorLayoutBinding{
    VkDescriptorSetLayoutBinding Binding;
    uint32_t SetIndex;
    size_t Size;
    std::string Name;

    bool operator<(const DescriptorLayoutBinding &r)const{
      if(SetIndex!=r.SetIndex) return SetIndex<r.SetIndex;
      if(Binding.binding!=r.Binding.binding) return Binding.binding<r.Binding.binding;
      if(Name!=r.Name) return Name<r.Name;
      if(Binding.descriptorType!=r.Binding.descriptorType) return Binding.descriptorType<r.Binding.descriptorType;
      if(Size!=r.Size) return Size<r.Size;

      return Binding.descriptorCount<r.Binding.descriptorCount;
    }

    bool operator==(const DescriptorLayoutBinding &r)const{
      return SetIndex==r.SetIndex&&
        Binding.binding!=r.Binding.binding&&
        Name==r.Name&&
        Size==r.Size&&
        Binding.descriptorType==r.Binding.descriptorType;
    }
  };

  using DescriptorTypeCounts=std::map<VkDescriptorType,uint32_t>;

  static VkDeviceSize MemoryAlignment(VkDeviceSize size,VkDeviceSize alignment){
    return (size+alignment-1)&~(alignment-1);
  }
}