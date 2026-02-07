#include<stdexcept>
#include<algorithm>
#include<vulkan/vulkan.h>
#include"Swapchain2.h"
#include"Surface.h"
namespace Engine{

  void Swapchain2::CreateSwapchain(uint32_t swapIndex,VkSwapchainKHR oldSwapchain){
    auto &chain=mChains[swapIndex];
    chain.mFormat=VK_FORMAT_UNDEFINED;

    auto cap=mDevice->Physical().SurfaceCapabilities(mWindow);
    uint32_t neededFlags=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkPresentModeKHR workingPresentMode=VK_PRESENT_MODE_IMMEDIATE_KHR;

    if((cap.supportedUsageFlags&neededFlags)!=neededFlags)
      throw std::runtime_error("Needed flags are not supported by surface");

    for(auto presentMode:mDevice->Physical().SurfacePresentModes(mWindow)){
      if(presentMode==VK_PRESENT_MODE_MAILBOX_KHR)
        workingPresentMode=VK_PRESENT_MODE_MAILBOX_KHR;
    }


    for(auto f:mDevice->Physical().SurfaceFormat(mWindow)){
      if(f.format==VK_FORMAT_A2B10G10R10_UNORM_PACK32&&f.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
        chain.mFormat=f.format;
        chain.mColorspace=f.colorSpace;
        break;
      }
    }

    if(chain.mFormat==VK_FORMAT_UNDEFINED)
      throw std::runtime_error("Unable to find a valid image format");

    chain.mResolution=cap.currentExtent;

    chain.mImageCount=std::max(cap.minImageCount,mRequestedImageCount);
    chain.mImageCount=std::min(cap.maxImageCount,chain.mImageCount);

    VkSwapchainCreateInfoKHR info={
      .sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext=nullptr,
      .flags=0,
      .surface=mWindow->Surface(),
      .minImageCount=chain.mImageCount,
      .imageFormat=chain.mFormat,
      .imageColorSpace=chain.mColorspace,
      .imageExtent=chain.mResolution,
      .imageArrayLayers=1,
      .imageUsage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode=VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount=0,
      .pQueueFamilyIndices=nullptr,
      .preTransform=VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode=workingPresentMode,
      .clipped=VK_FALSE,
      .oldSwapchain=oldSwapchain
    };
    auto status=vkCreateSwapchainKHR(mDevice->Handle(),&info,nullptr,&chain.mHandle);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to create swapchain");
  }


  void Swapchain2::GetSwapchainImages(uint32_t swapIndex){
    auto &chain=mChains[swapIndex];
    VkExtent3D imageExtent={
      .width=chain.mResolution.width,
      .height=chain.mResolution.height,
      .depth=1
    };

    std::vector<VkImage> images;

    uint32_t imageCount=0;
    auto status=vkGetSwapchainImagesKHR(
      mDevice->Handle(),
      chain.mHandle,
      &imageCount,
      nullptr);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to get swapchain image count");

    images.resize(imageCount);

    status=vkGetSwapchainImagesKHR(
      mDevice->Handle(),
      chain.mHandle,
      &imageCount,
      images.data());
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to get swapchain images");

    chain.mImages.clear();
    for(auto image:images)
      chain.mImages.push_back(SwapchainImage::Create(mDevice,image,VK_IMAGE_TYPE_2D,chain.mFormat,imageExtent));


    for(auto &img:chain.mImages)
      chain.mImageViews.push_back(img->CreateImageView());
  }

  void Swapchain2::CreateFences(uint32_t swapIndex){
    auto &chain=mChains[swapIndex];

    VkFenceCreateInfo fenceInfo={
      .sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext=nullptr,
      .flags=VK_FENCE_CREATE_SIGNALED_BIT
    };
    for(size_t i=0;i<chain.mImages.size();i++){
      VkFence fence;
      vkCreateFence(mDevice->Handle(),&fenceInfo,nullptr,&fence);
      chain.mFences.push_back(fence);
    }
  }

  void Swapchain2::DestroyFences(uint32_t swapIndex){
    auto &chain=mChains[swapIndex];

    if(chain.mFences.empty())
      return;

    vkWaitForFences(mDevice->Handle(),static_cast<uint32_t>(chain.mFences.size()),chain.mFences.data(),VK_TRUE,UINT64_MAX);

    for(auto fence:chain.mFences)
      vkDestroyFence(mDevice->Handle(),fence,nullptr);
    chain.mFences.clear();
  }

  VkFence Swapchain2::GetNextFence(uint32_t swapIndex){
    auto &chain=mChains[swapIndex];

    for(auto fence:chain.mFences){
      VkResult status=vkGetFenceStatus(mDevice->Handle(),fence);
      if(status==VK_SUCCESS)
        return fence;
    }

    throw std::runtime_error("No available fence in swapchain images");
  }


  bool Swapchain2::OutstandingFences(uint32_t swapIndex){
    auto &chain=mChains[swapIndex];
    for(auto fence:chain.mFences){
      VkResult status=vkGetFenceStatus(mDevice->Handle(),fence);
      if(status==VK_NOT_READY)
        return true;
    }
    return false;
  }

  void Swapchain2::CleanupRetired(){
    uint32_t oldChainIndex=mCurrentChain==0?1:0;
    auto &oldChain=mChains[oldChainIndex];
    bool fencesInUse=false;
    for(auto fence:oldChain.mFences){
      VkResult status=vkGetFenceStatus(mDevice->Handle(),fence);
      if(status==VK_NOT_READY){
        fencesInUse=true;
        break;
      }
    }

    if(!fencesInUse){
      DestroyFences(oldChainIndex);
      vkDestroySwapchainKHR(mDevice->Handle(),oldChain.mHandle,nullptr);
      oldChain.mHandle=VK_NULL_HANDLE;
      oldChain.mImages.clear();
      oldChain.mImageViews.clear();
    }
  }

  Swapchain2::Swapchain2(DevicePtr &device,WindowPtr &window,uint32_t imageCount):
    mDevice(device),mWindow(window),mRequestedImageCount(imageCount){
    mCurrentChain=0;

    CreateSwapchain(mCurrentChain,VK_NULL_HANDLE);
    GetSwapchainImages(mCurrentChain);
    CreateFences(mCurrentChain);
  }

  Swapchain2::~Swapchain2(){
    DestroyFences(0);
    DestroyFences(1);

    for(auto &chain:mChains){
      if(chain.mHandle)
        vkDestroySwapchainKHR(mDevice->Handle(),chain.mHandle,nullptr);

      chain.mImages.clear();
      chain.mImageViews.clear();
    }
  }

  std::tuple<SwapchainImagePtr,ImageViewPtr> Swapchain2::Next(){
    CleanupRetired();
    uint32_t imageIndex=0;

    auto fence=GetNextFence(mCurrentChain);
    vkResetFences(mDevice->Handle(),1,&fence);

    auto result=vkAcquireNextImageKHR(
      mDevice->Handle(),
      mChains[mCurrentChain].mHandle,
      UINT64_MAX,
      VK_NULL_HANDLE,
      fence,
      &imageIndex);

    auto image=mChains[mCurrentChain].mImages[imageIndex];
    auto imageView=mChains[mCurrentChain].mImageViews[imageIndex];

    if(result==VK_SUBOPTIMAL_KHR){
    } else if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to acquire next swapchain image");

    //if(!image->Ready())
    //  throw std::runtime_error("Desynchronization of Swapchain fences");

    image->SetFence(fence);

    vkWaitForFences(mDevice->Handle(),1,&fence,VK_TRUE,UINT64_MAX);

    return std::make_tuple(image,imageView);
  }

  void Swapchain2::PresentImage(QueuePtr &queue,SwapchainImagePtr &image){
    CleanupRetired();
    auto &chain=mChains[mCurrentChain];
    uint32_t imageIndex=0;
    uint32_t chainIndex=mCurrentChain;
    VkSwapchainKHR swapchain=chain.mHandle;

    auto findImageIndex=[&](const Chain &c,SwapchainImagePtr &img,uint32_t &outIndex)->bool{
      for(uint32_t i=0;i<c.mImages.size();i++){
        if(c.mImages[i]==img){
          outIndex=i;
          return true;
        }
      }
      return false;
    };

    if(!findImageIndex(chain,image,imageIndex)){
      chainIndex=mCurrentChain==0?1:0;
      chain=mChains[chainIndex];
      swapchain=chain.mHandle;
      
      if(!findImageIndex(chain,image,imageIndex))
        throw std::runtime_error("The provided image does not belong to the current swapchains");
    }

    VkPresentInfoKHR info={
      .sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext=nullptr,
      .waitSemaphoreCount=0,
      .pWaitSemaphores=nullptr,
      .swapchainCount=1,
      .pSwapchains=&swapchain,
      .pImageIndices=&imageIndex,
      .pResults=nullptr
    };
    auto status=vkQueuePresentKHR(queue->Handle(),&info);
    
    if(status==VK_SUBOPTIMAL_KHR){
      vkQueueWaitIdle(queue->Handle());

      if(!OutstandingFences(mCurrentChain)){
        DestroyFences(mCurrentChain);
        vkDestroySwapchainKHR(mDevice->Handle(),mChains[mCurrentChain].mHandle,nullptr);
        mChains[mCurrentChain].mHandle=VK_NULL_HANDLE;
        mChains[mCurrentChain].mImages.clear();
        mChains[mCurrentChain].mImageViews.clear();

        mCurrentChain=mCurrentChain==0?1:0;
        CreateSwapchain(mCurrentChain,nullptr);
        GetSwapchainImages(mCurrentChain);
        CreateFences(mCurrentChain);
      }
    }else if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to present swapchain image");
  }
}