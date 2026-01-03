#include<stdexcept>
#define VK_USE_PLATFORM_WIN32_KHR
#include "Swapchain.h"


Swapchain::Swapchain(DevicePtr device,SurfacePtr surface):mDevice(device),mSurface(surface){
  mHandle=CreateSwapchain(VK_NULL_HANDLE);
  AllocateImages();

  VkFenceCreateInfo fenceInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0 // unsignaled fence
  };

  if (vkCreateFence(mDevice->Handle(), &fenceInfo, nullptr, &mAcquireFence) != VK_SUCCESS)
    throw std::runtime_error("Unable to create acquire fence");
}

Swapchain::~Swapchain(){
  DestroyImageViews();
  vkDestroySwapchainKHR(mDevice->Handle(),mHandle,nullptr);
  vkDestroyFence(mDevice->Handle(), mAcquireFence, nullptr);
}

VkSwapchainKHR Swapchain::CreateSwapchain(VkSwapchainKHR oldswap){
  VkSwapchainKHR handle;
  auto cap=mDevice->Physical().SurfaceCapabilities(mSurface);
  uint32_t neededFlags=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  VkPresentModeKHR presentMode=VK_PRESENT_MODE_IMMEDIATE_KHR;

  if((cap.supportedUsageFlags&neededFlags)!=neededFlags)
    throw std::runtime_error("Needed flags are not supported by surface");

  for(auto presentMode:mDevice->Physical().SurfacePresentModes(mSurface)){
    if(presentMode==VK_PRESENT_MODE_MAILBOX_KHR)
      presentMode=VK_PRESENT_MODE_MAILBOX_KHR;
  }


  for(auto f:mDevice->Physical().SurfaceFormat(mSurface)){
    if(f.format==VK_FORMAT_A2B10G10R10_UNORM_PACK32&&f.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
      mFormat=f.format;
      mColorSpace=f.colorSpace;
      break;
    }
  }

  if(mFormat==VK_FORMAT_UNDEFINED)
    throw std::runtime_error("Unable to find a valid image format");
  
  mResolution=cap.currentExtent;
  
  VkSwapchainCreateInfoKHR info={
    .sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext=nullptr,
    .flags=0,
    .surface=mSurface->Handle(),
    .minImageCount=cap.minImageCount,
    .imageFormat=mFormat,
    .imageColorSpace=mColorSpace,
    .imageExtent=mResolution,
    .imageArrayLayers=1,
    .imageUsage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode=VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount=0,
    .pQueueFamilyIndices=nullptr,
    .preTransform=VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    .compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode=VK_PRESENT_MODE_IMMEDIATE_KHR,
    .clipped=VK_FALSE,
    .oldSwapchain=oldswap
  };
  auto status=vkCreateSwapchainKHR(mDevice->Handle(),&info,nullptr,&handle);
  if(status!=VK_SUCCESS)
    throw std::runtime_error("Unable to create swapchain");

  return handle;
}

void Swapchain::AllocateImages(){
  DestroyImageViews();
  
  VkExtent3D imageExtent={
    .width=mResolution.width,
    .height=mResolution.height,
    .depth=1
  };

  std::vector<VkImage> images;
   
  uint32_t imageCount=0;
  auto status=vkGetSwapchainImagesKHR(
    mDevice->Handle(),
    mHandle,
    &imageCount,
    nullptr);
  if(status!=VK_SUCCESS)
    throw std::runtime_error("Unable to get swapchain image count");

  images.resize(imageCount);
  
  status=vkGetSwapchainImagesKHR(
    mDevice->Handle(),
    mHandle,
    &imageCount,
    images.data());
  if(status!=VK_SUCCESS)
    throw std::runtime_error("Unable to get swapchain images");


  for(auto image:images){
    mOldLayouts.push_back(VK_IMAGE_LAYOUT_UNDEFINED);

    mImages.push_back(SwapchainImage::Create(mDevice,image,VK_IMAGE_TYPE_2D,mFormat,imageExtent));
    mImageViews.push_back(mImages.back()->CreateImageView());
  }
}

void Swapchain::Rebuild(){
  mHandle=CreateSwapchain(mHandle);
  AllocateImages();
}

void Swapchain::DestroyImageViews(){
  mImageViews.clear();
  mOldLayouts.clear();
}

uint32_t Swapchain::AcquireNextImage(){
  uint32_t imageIndex=0;
  
  auto status=vkAcquireNextImageKHR(
    mDevice->Handle(),
    mHandle,
    UINT64_MAX,
    VK_NULL_HANDLE,
    mAcquireFence,
    &imageIndex);

  if(status!=VK_SUCCESS && status!=VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("Unable to acquire next swapchain image");


  auto waitStatus = vkWaitForFences(mDevice->Handle(), 1, &mAcquireFence, VK_TRUE, UINT64_MAX);
  if (waitStatus != VK_SUCCESS)
    throw std::runtime_error("Failed waiting for acquire fence");

  // Reset the fence for reuse
  vkResetFences(mDevice->Handle(), 1, &mAcquireFence);

  mCurrentImageIndex=imageIndex;
  return imageIndex;
}

void Swapchain::PresentImage(QueuePtr &queue){
  VkPresentInfoKHR info={
    .sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext=nullptr,
    .waitSemaphoreCount=0,
    .pWaitSemaphores=nullptr,
    .swapchainCount=1,
    .pSwapchains=&mHandle,
    .pImageIndices=&mCurrentImageIndex,
    .pResults=nullptr
  };
  auto status=vkQueuePresentKHR(queue->Handle(),&info);
  if(status!=VK_SUCCESS)
    throw std::runtime_error("Unable to present swapchain image");
}

void Swapchain::TransitionSwapchain(VkCommandBuffer cmdBuffer,
  VkPipelineStageFlags2 srcStage,VkAccessFlags2 srcAccess,
  VkPipelineStageFlags2 dstStage,VkAccessFlags2 dstAccess,VkImageLayout newLayout){

  VkImageMemoryBarrier2 barrier={
    .sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .pNext=nullptr,
    .srcStageMask=srcStage,
    .srcAccessMask=srcAccess,
    .dstStageMask=dstStage,
    .dstAccessMask=dstAccess,
    .oldLayout=mOldLayouts[mCurrentImageIndex],
    .newLayout=newLayout,
    .srcQueueFamilyIndex=mDevice->GetPrimiaryQueueFamily(),
    .dstQueueFamilyIndex=mDevice->GetPrimiaryQueueFamily(),
    .image=mImages[mCurrentImageIndex]->Handle(),
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
  mOldLayouts[mCurrentImageIndex]=newLayout;
}

void Swapchain::GetDescriptor(void *descriptor,VkDescriptorType type){
  PFN_vkGetDescriptorEXT pfnGetDescriptorEXT=reinterpret_cast<PFN_vkGetDescriptorEXT>(
    vkGetDeviceProcAddr(mDevice->Handle(),"vkGetDescriptorEXT"));

  VkDescriptorDataEXT data={nullptr};
  VkDescriptorImageInfo imageInfo={};
  size_t size=0;
  auto properties=mDevice->GetDescriptorProperties();

  switch(type){
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      imageInfo={
        .sampler=VK_NULL_HANDLE,
        .imageView=mImageViews[mCurrentImageIndex]->Handle(),
        .imageLayout=mOldLayouts[mCurrentImageIndex]
      };
      data.pStorageImage=&imageInfo;
      size=properties.storageImageDescriptorSize;
      break;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      imageInfo={
        .sampler=VK_NULL_HANDLE,
        .imageView=mImageViews[mCurrentImageIndex]->Handle(),
        .imageLayout=mOldLayouts[mCurrentImageIndex]
      };
      data.pInputAttachmentImage=&imageInfo;
      size=properties.inputAttachmentDescriptorSize;
      break;
  }

  VkDescriptorGetInfoEXT descriptorGetInfo={
    .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
    .pNext=nullptr,
    .type=type,
    .data=data
  };

  pfnGetDescriptorEXT(mDevice->Handle(),&descriptorGetInfo,size,descriptor);
}