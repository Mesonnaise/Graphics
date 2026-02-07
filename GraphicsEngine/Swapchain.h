#pragma once
#include<memory>
#include<array>
#include<vulkan/vulkan.h>
#include"Device.h"
#include"Window.h"
#include"Image.h"
#include"ImageView.h"

namespace Engine{
  class Swapchain:public std::enable_shared_from_this<Swapchain>{
    DevicePtr mDevice=nullptr;
    WindowPtr mWindow=nullptr;
    VkSwapchainKHR mHandle=nullptr;
    
    std::array<VkFence,2> mImageAcquireFence;

    VkFence mAcquireFence=nullptr;

    VkFormat mFormat=VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR mColorSpace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkExtent2D mResolution{0,0};
    uint32_t mCurrentImageIndex=0;


    std::vector<SwapchainImagePtr> mImages;
    std::vector<ImageViewPtr> mImageViews;
    std::vector<VkImageLayout> mOldLayouts;

  protected:
    Swapchain(DevicePtr device,WindowPtr window);

    VkSwapchainKHR CreateSwapchain(VkSwapchainKHR oldswap);
    void AllocateImages();
    void DestroyImageViews();
  public:
    static auto Create(DevicePtr device,WindowPtr window){
      auto p=new Swapchain(device,window);
      return std::shared_ptr<Swapchain>(p);
    }
    ~Swapchain();
    void Rebuild();
    uint32_t AcquireNextImage();
    constexpr VkSwapchainKHR Handle()const{
      return mHandle;
    }
    inline SwapchainImagePtr GetImage(uint32_t index){
      return mImages.at(index);
    }
    inline ImageViewPtr GetImageView(uint32_t index){
      return mImageViews.at(index);
    }

    void PresentImage(QueuePtr &queue);
    void TransitionSwapchain(
      VkCommandBuffer cmdBuffer,
      VkPipelineStageFlags2 srcStage,VkAccessFlags2 srcAccess,
      VkPipelineStageFlags2 dstStage,VkAccessFlags2 dstAccess,VkImageLayout newLayout);

    void GetDescriptor(void *descriptor,VkDescriptorType type);
  };

  using SwapchainPtr=std::shared_ptr<Swapchain>;
}

class Swapchain{};
