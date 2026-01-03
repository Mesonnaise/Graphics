#include<stdexcept>
#include"Image.h"
#include"ImageView.h"
#include"Allocator.h"
static PFN_vkGetDescriptorEXT pfnGetDescriptorEXT=nullptr;
static void LoadExtensions(VkDevice device){
  if(pfnGetDescriptorEXT==nullptr){
    pfnGetDescriptorEXT=reinterpret_cast<PFN_vkGetDescriptorEXT>(
      vkGetDeviceProcAddr(device,"vkGetDescriptorEXT"));
  }
}

BaseImage::BaseImage(DevicePtr &device):Resource(device){}

ImageViewPtr BaseImage::CreateImageView(){
  return ImageView::Create(mDevice,std::dynamic_pointer_cast<BaseImage>(shared_from_this()));
}


Image::Image(DevicePtr &device,VkExtent3D extent,VkFormat format,VkImageUsageFlags usage):BaseImage(device){
  mImage=VK_NULL_HANDLE;
  mFormat=format;
  mExtent=extent;
  mUsage=usage;

  mType=VK_IMAGE_TYPE_2D;
  if(extent.height==1){
    mType=VK_IMAGE_TYPE_1D;
    if(extent.depth!=1)
      throw std::runtime_error("1D image with depth geater than 1 is not supported");
  }else if(extent.depth>1)
    mType=VK_IMAGE_TYPE_3D;

  VkImageCreateInfo info={
    .sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext=nullptr,
    .flags=0,
    .imageType=mType,
    .format=mFormat,
    .extent=mExtent,
    .mipLevels=1,
    .arrayLayers=1,
    .samples=VK_SAMPLE_COUNT_1_BIT,
    .tiling=VK_IMAGE_TILING_OPTIMAL,
    .usage=mUsage,
    .sharingMode=VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount=0,
    .pQueueFamilyIndices=nullptr,
    .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED
  };

  auto result=vkCreateImage(mDevice->Handle(),&info,nullptr,&mImage);
  if(result!=VK_SUCCESS)
    throw std::runtime_error("Unable to create image");
}

Image::~Image(){
  mAllocator->FreeAllocation(mAllocation);
  vkDestroyImage(mDevice->Handle(),mImage,nullptr);
  mImage=VK_NULL_HANDLE;
}

VkMemoryRequirements BaseImage::MemoryRequirements(){
  VkImageMemoryRequirementsInfo2 info={
    .sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
    .pNext=nullptr,
    .image=mImage
  };

  VkMemoryRequirements2 requirements{};
  vkGetImageMemoryRequirements2(mDevice->Handle(),&info,&requirements);
  return requirements.memoryRequirements;
}


SwapchainImage::SwapchainImage(DevicePtr &device,VkImage image,VkImageType imageType,VkFormat format,VkExtent3D extent):BaseImage(device){
  mType=imageType;
  mImage=image;
  mFormat=format;
  mExtent=extent;
  mUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT;
}

SwapchainImage::~SwapchainImage(){};