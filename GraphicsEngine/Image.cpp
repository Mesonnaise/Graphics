#include<stdexcept>
#include"Image.h"
#include"ImageView.h"
#include"Allocator.h"

namespace Engine{
  BaseImage::BaseImage(DevicePtr &device):Resource(device){}

  ImageViewPtr BaseImage::CreateImageView(){
    return ImageView::Create(mDevice,std::dynamic_pointer_cast<BaseImage>(shared_from_this()));
  }


  void BaseImage::TransitionLayout(VkCommandBuffer cmdBuffer,
    VkPipelineStageFlags2 srcStage,VkAccessFlags2 srcAccess,
    VkPipelineStageFlags2 dstStage,VkAccessFlags2 dstAccess,VkImageLayout newLayout){

    VkImageMemoryBarrier2 barrier={
      .sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .pNext=nullptr,
      .srcStageMask=srcStage,
      .srcAccessMask=srcAccess,
      .dstStageMask=dstStage,
      .dstAccessMask=dstAccess,
      .oldLayout=mCurrentLayout,
      .newLayout=newLayout,
      .srcQueueFamilyIndex=mDevice->GetPrimiaryQueueFamily(),
      .dstQueueFamilyIndex=mDevice->GetPrimiaryQueueFamily(),
      .image=mImage,
      .subresourceRange={
        .aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel=0,
        .levelCount=1,
        .baseArrayLayer=0,
        .layerCount=1
      }
    };
    VkDependencyInfo info={
      .sType=VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .pNext=nullptr,
      .dependencyFlags=0,
      .memoryBarrierCount=0,
      .pMemoryBarriers=nullptr,
      .bufferMemoryBarrierCount=0,
      .pBufferMemoryBarriers=nullptr,
      .imageMemoryBarrierCount=1,
      .pImageMemoryBarriers=&barrier
    };
    vkCmdPipelineBarrier2(cmdBuffer,&info);
    mCurrentLayout=newLayout;
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
    } else if(extent.depth>1)
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

  bool SwapchainImage::Ready(){
    if(mFence==nullptr)
      return true;

    auto result=vkGetFenceStatus(mDevice->Handle(),mFence);
    if(result==VK_ERROR_DEVICE_LOST)
      throw std::runtime_error("Device lost while checking fence status");
    return result==VK_SUCCESS;
  }

  void SwapchainImage::Wait(){
    auto result=vkWaitForFences(mDevice->Handle(),1,&mFence,VK_TRUE,UINT64_MAX);
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Failed waiting for swapchain image fence");
  }
}