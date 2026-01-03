#pragma once
#include<memory>
#include<vulkan/vulkan.h>
#include"Device.h"

class Allocator;

class Resource:public std::enable_shared_from_this<Resource>{
  friend class Allocator;
protected:
  DevicePtr mDevice=nullptr;
  std::shared_ptr<Allocator> mAllocator=nullptr;
  void *mAllocation=nullptr;

  enum class ResourceType{
    Buffer,
    DescriptorBuffer,
    Image
  } mType;
  void *mDataPointer=nullptr;
  //MemoryHandle;

  union{
    VkBuffer mBuffer=nullptr;
    VkImage mImage;
  };

protected:
  Resource(DevicePtr &device);
  void AssignAllocator(std::shared_ptr<Allocator> allocator,void *allocation,void *data);
public:
  virtual ~Resource()=0;
  virtual VkDeviceSize AllocatedSize();
  virtual VkDeviceSize AllocatedOffset();
  virtual VkDeviceAddress DeviceAddress();
  virtual void *Mapped();
  virtual VkMemoryRequirements MemoryRequirements()=0;
};
