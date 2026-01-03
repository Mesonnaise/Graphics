#include<stdexcept>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include<vma/vk_mem_alloc.h>
#include "Allocator.h"

namespace Engine{
  static VmaAllocationCreateInfo CreateAllocationInfo(bool hostVisible){
    VmaAllocationCreateInfo allocateInfo={
      .flags=0,
      .usage=VMA_MEMORY_USAGE_UNKNOWN,
      .requiredFlags=0,
      .preferredFlags=0,
      .memoryTypeBits=0,
      .pool=nullptr,
      .pUserData=nullptr,
      .priority=0.0f
    };
    if(hostVisible){
      allocateInfo.requiredFlags|=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      allocateInfo.preferredFlags|=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      allocateInfo.flags|=VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT|VMA_ALLOCATION_CREATE_MAPPED_BIT;
    } else{
      allocateInfo.requiredFlags|=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    allocateInfo.requiredFlags=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return allocateInfo;
  }


  static uint32_t findMemoryType(uint32_t typeFilter,VkMemoryPropertyFlags properties,
    const VkPhysicalDeviceMemoryProperties &memProperties){
    for(uint32_t i=0; i<memProperties.memoryTypeCount; i++){
      if((typeFilter&(1<<i))&&(memProperties.memoryTypes[i].propertyFlags&properties)==properties){
        return i;
      }
    }
    // Handle error: No suitable memory type found
    return 0; // Or throw exception
  }

  Allocator::Allocator(InstancePtr &instance,DevicePtr &device,bool perferDedicated):mDevice(device){
    VmaVulkanFunctions vulkanFunctions={
    .vkGetInstanceProcAddr=&vkGetInstanceProcAddr,
    .vkGetDeviceProcAddr=&vkGetDeviceProcAddr,
    };

    VmaAllocatorCreateInfo info={
      .flags=VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT|VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT,
      .physicalDevice=device->Physical().Handle(),
      .device=device->Handle(),
      .preferredLargeHeapBlockSize=0,
      .pAllocationCallbacks=nullptr,
      .pDeviceMemoryCallbacks=nullptr,
      .pHeapSizeLimit=nullptr,
      .pVulkanFunctions=&vulkanFunctions,
      .instance=instance->Handle(),
      .vulkanApiVersion=VK_API_VERSION_1_4,
      .pTypeExternalMemoryHandleTypes=nullptr
    };


    auto result=vmaCreateAllocator(&info,reinterpret_cast<VmaAllocator *>(&mAllocator));
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to create memory allocator");
  }

  Allocator::~Allocator(){
    vmaDestroyAllocator((VmaAllocator)mAllocator);
  }

  std::tuple<VkDeviceSize,VkDeviceSize> Allocator::AllocationInfo(void *allocation){
    VmaAllocationInfo info={};
    vmaGetAllocationInfo((VmaAllocator)mAllocator,(VmaAllocation)allocation,&info);
    return {info.size,info.offset};
  }

  void Allocator::FreeAllocation(void *allocation){
    vmaFreeMemory((VmaAllocator)mAllocator,(VmaAllocation)allocation);
  }

  BufferPtr Allocator::CreateBuffer(VkDeviceSize size,VkBufferUsageFlags usage,bool hostVisable){
    BufferPtr buffer=Buffer::Create(mDevice,size,usage);

    /*
    uint32_t filter=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto requirements=buffer->MemoryRequirements();
    auto memoryProperties=mDevice->Physical().MemoryProperties();
    auto memoryType=findMemoryType(requirements.memoryTypeBits,filter,memoryProperties);

    memoryProperties.memoryTypes;

    VkDeviceMemory memory;
    VkMemoryAllocateInfo info={
      .sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext=nullptr,
      .allocationSize=requirements.size,
      .memoryTypeIndex=memoryType
    };

    auto result=vkAllocateMemory(mDevice->Handle(),&info,nullptr,&memory);
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to allocate memory");

    VkBindBufferMemoryInfo bind={
      .sType=VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
      .pNext=nullptr,
      .buffer=buffer->Handle(),
      .memory=memory,
      .memoryOffset=0,
    };

    vkBindBufferMemory2(mDevice->Handle(),1,&bind);
    */

    VmaAllocation allocation=nullptr;
    VmaAllocationInfo allocInfoOut={};

    VmaAllocationCreateInfo allocateInfo=CreateAllocationInfo(hostVisable);

    auto result=vmaAllocateMemoryForBuffer((VmaAllocator)mAllocator,buffer->Handle(),&allocateInfo,&allocation,&allocInfoOut);
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to allocate memory for buffer");

    vmaBindBufferMemory((VmaAllocator)mAllocator,allocation,buffer->Handle());

    buffer->AssignAllocator(shared_from_this(),allocation,allocInfoOut.pMappedData);

    return buffer;
  }

  DescriptorBufferPtr Allocator::CreateDescriptorBuffer(VkDeviceSize size,bool hostVisable){
    VkBufferUsageFlags usage=VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT|VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    if(!hostVisable)
      usage|=VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    DescriptorBufferPtr buffer=DescriptorBuffer::Create(mDevice,size,usage);

    VmaAllocation allocation=nullptr;
    VmaAllocationInfo allocInfoOut={};

    VmaAllocationCreateInfo allocateInfo=CreateAllocationInfo(hostVisable);

    auto result=vmaAllocateMemoryForBuffer((VmaAllocator)mAllocator,buffer->Handle(),&allocateInfo,&allocation,&allocInfoOut);
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to allocate memory for buffer");

    vmaBindBufferMemory((VmaAllocator)mAllocator,allocation,buffer->Handle());

    buffer->AssignAllocator(shared_from_this(),allocation,allocInfoOut.pMappedData);

    return buffer;
  }

  ImagePtr Allocator::CreateImage(VkExtent3D extent,VkFormat format,VkImageUsageFlags usage,bool hostVisable){
    ImagePtr image=Image::Create(mDevice,extent,format,usage);

    //auto requirements=image->MemoryRequirements();
    VmaAllocation allocation=nullptr;
    VmaAllocationInfo allocInfoOut={};
    VmaAllocationCreateInfo allocateInfo=CreateAllocationInfo(hostVisable);


    auto result=vmaAllocateMemoryForImage((VmaAllocator)mAllocator,image->Handle(),&allocateInfo,&allocation,&allocInfoOut);
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to allocate memory for image");

    vmaBindImageMemory((VmaAllocator)mAllocator,allocation,image->Handle());
    image->AssignAllocator(shared_from_this(),allocation,allocInfoOut.pMappedData);
    return image;
  }
}