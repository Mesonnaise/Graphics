#include<stdexcept>
#include "Buffer.h"
#include"Allocator.h"

static PFN_vkGetDescriptorEXT pfnGetDescriptorEXT=nullptr;
static PFN_vkCmdBindDescriptorBuffersEXT pfnCmdBindDescriptorBuffersEXT=nullptr;

static void LoadExtensions(VkDevice device){
  if(pfnGetDescriptorEXT==nullptr){
    pfnCmdBindDescriptorBuffersEXT=reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(
      vkGetDeviceProcAddr(device,"vkCmdBindDescriptorBuffersEXT"));
    pfnGetDescriptorEXT=reinterpret_cast<PFN_vkGetDescriptorEXT>(
      vkGetDeviceProcAddr(device,"vkGetDescriptorEXT"));
  }
}

static VkDescriptorType MapDescriptorType(VkBufferUsageFlags flags){
  switch(flags){
    case VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT:
      return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT:
      return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    default:
      throw std::runtime_error("Unsupported buffer usage for descriptor.");
  }
}

Buffer::Buffer(DevicePtr &device,VkDeviceSize size,VkBufferUsageFlags usage):Resource(device),mUsage(usage){
  LoadExtensions(mDevice->Handle());
  mType=ResourceType::Buffer;

  VkBufferCreateInfo info={
    .sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext=nullptr,
    .flags=0,
    .size=size,
    .usage=usage,
    .sharingMode=VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount=0,
    .pQueueFamilyIndices=nullptr
  };
  vkCreateBuffer(mDevice->Handle(),&info,nullptr,&mBuffer);

  
}

Buffer::~Buffer(){
  mAllocator->FreeAllocation(mAllocation);
  vkDestroyBuffer(mDevice->Handle(),mBuffer,nullptr);
  mBuffer=nullptr;

}

VkDeviceAddress Buffer::DeviceAddress(){
  VkBufferDeviceAddressInfo bufferDeviceAddressInfo={
  .sType=VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
  .pNext=nullptr,
  .buffer=mBuffer
  };
  return vkGetBufferDeviceAddress(mDevice->Handle(),&bufferDeviceAddressInfo);
}

VkMemoryRequirements Buffer::MemoryRequirements(){
  VkBufferMemoryRequirementsInfo2 info={
    .sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
    .pNext=nullptr,
    .buffer=mBuffer
  };

  VkMemoryRequirements2 requirements{};
  vkGetBufferMemoryRequirements2(mDevice->Handle(),&info,&requirements);
  return requirements.memoryRequirements;
}

size_t Buffer::GetDescriptor(void *WritePointer,VkDescriptorType descriptorType){
  auto propertries=mDevice->GetDescriptorProperties();
  size_t descriptorSize=0;
  switch(descriptorType){
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      descriptorSize=propertries.uniformTexelBufferDescriptorSize;
      break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      descriptorSize=propertries.storageTexelBufferDescriptorSize;
      break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      descriptorSize=propertries.uniformBufferDescriptorSize;
      break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      descriptorSize=propertries.storageBufferDescriptorSize;
      break;
    default:
      throw std::runtime_error("Unsupported buffer usage for descriptor.");
  }

  VkDescriptorAddressInfoEXT addressInfo={
    .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
    .pNext=nullptr,
    .address=DeviceAddress(),
    .range=AllocatedSize(),
    .format=VK_FORMAT_UNDEFINED
  };

  VkDescriptorGetInfoEXT descriptorInfo={
    .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
    .pNext=nullptr,
    .type=descriptorType,
    .data={
      .pUniformBuffer=&addressInfo
    }
  };

  pfnGetDescriptorEXT(
    mDevice->Handle(),
    &descriptorInfo,
    descriptorSize,
    WritePointer
  );

  return descriptorSize;
}

DescriptorBuffer::DescriptorBuffer(DevicePtr &device,VkDeviceSize size,VkBufferUsageFlags usage):Buffer(device,size,usage){}

void DescriptorBuffer::CommandBufferBind(VkCommandBuffer cmdBuffer){
  VkDescriptorBufferBindingInfoEXT bufferBindingInfo={
    .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
    .pNext=nullptr,
    .address=DeviceAddress(),
    .usage=VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
  };

  pfnCmdBindDescriptorBuffersEXT(cmdBuffer,1,&bufferBindingInfo);
}