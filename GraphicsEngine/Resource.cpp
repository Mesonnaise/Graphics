#include<stdexcept>
#include "Resource.h"
#include"Allocator.h"

namespace Engine{
  static PFN_vkGetDescriptorEXT pfnGetDescriptorEXT=nullptr;

  void LoadExtensions(VkDevice device){
    if(pfnGetDescriptorEXT==nullptr){
      pfnGetDescriptorEXT=reinterpret_cast<PFN_vkGetDescriptorEXT>(
        vkGetDeviceProcAddr(device,"vkGetDescriptorEXT"));
    }
  }

  Resource::Resource(DevicePtr &device):mDevice(device),mBuffer(nullptr){}
  Resource::~Resource(){}

  VkDeviceSize Resource::AllocatedSize(){
    auto [size,offset]=mAllocator->AllocationInfo(mAllocation);
    return size;
  }

  VkDeviceSize Resource::AllocatedOffset(){
    auto [size,offset]=mAllocator->AllocationInfo(mAllocation);
    return offset;
  }

  void *Resource::Mapped(){
    return mDataPointer;
  }

  VkDeviceAddress Resource::DeviceAddress(){
    throw std::runtime_error("Buffer device address not support.");
    return 0;
  }

  void Resource::AssignAllocator(std::shared_ptr<Allocator> allocator,void *allocation,void *data){
    mAllocator=allocator;
    mAllocation=allocation;
    mDataPointer=data;
  }
}