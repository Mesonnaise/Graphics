#pragma once
#include<memory>
#include<tuple>
#include<vulkan/vulkan.h>

#include"Device.h"
#include"Buffer.h"
#include"Image.h"
class Allocator:public std::enable_shared_from_this<Allocator>{
  friend class Resource;
  friend class Image;
  friend class Buffer;
  DevicePtr mDevice=nullptr;

  void *mAllocator=nullptr;

protected:
  Allocator(InstancePtr &instance,DevicePtr &device,bool perferDedicated);

  void FreeAllocation(void *allocation);
public:
  static inline std::shared_ptr<Allocator> Create(InstancePtr &instance,DevicePtr &device,bool perferDedicated=false){
    auto p=new Allocator(instance,device,perferDedicated);
    return std::shared_ptr<Allocator>(p);
  }

  ~Allocator();
  

  BufferPtr CreateBuffer(VkDeviceSize size,VkBufferUsageFlags usage,bool hostVisable=false);
  DescriptorBufferPtr CreateDescriptorBuffer(VkDeviceSize size,bool hostVisable=false);
  ImagePtr CreateImage(VkExtent3D extent,VkFormat format,VkImageUsageFlags usage,bool hostVisable=false);

  std::tuple<VkDeviceSize,VkDeviceSize> AllocationInfo(void *allocation);
};

using AllocatorPtr=std::shared_ptr<Allocator>;