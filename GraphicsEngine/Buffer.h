#pragma once
#include<memory>
#include "Resource.h"

namespace Engine{
  class Allocator;

  class Buffer:public Resource,public std::enable_shared_from_this<Buffer>{
    friend class Allocator;
    VkBufferUsageFlags mUsage;

  protected:
    Buffer(DevicePtr &device,VkDeviceSize size,VkBufferUsageFlags usage);

  public:
    static std::shared_ptr<Buffer> Create(DevicePtr &device,VkDeviceSize size,VkBufferUsageFlags usage){
      auto p=new Buffer(device,size,usage);
      return std::shared_ptr<Buffer>(p);
    }
    ~Buffer() override;

    constexpr VkBuffer Handle(){
      return mBuffer;
    }

    VkDeviceAddress DeviceAddress() override;
    VkMemoryRequirements MemoryRequirements() override;

    size_t GetDescriptor(void *WritePointer,VkDescriptorType descriptorType);
  };

  class DescriptorBuffer:public Buffer{


  protected:
    DescriptorBuffer(DevicePtr &device,VkDeviceSize size,VkBufferUsageFlags usage);
  public:
    static std::shared_ptr<DescriptorBuffer> Create(DevicePtr &device,VkDeviceSize size,VkBufferUsageFlags usage){
      auto p=new DescriptorBuffer(device,size,usage);
      return std::shared_ptr<DescriptorBuffer>(p);
    }

    void CommandBufferBind(VkCommandBuffer CMDBuffer);
  };

  using BufferPtr=std::shared_ptr<Buffer>;
  using DescriptorBufferPtr=std::shared_ptr<DescriptorBuffer>;
}