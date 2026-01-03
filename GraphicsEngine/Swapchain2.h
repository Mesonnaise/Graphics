#pragma once
#include<memory>
#include<cstdint>
#include<array>
#include<tuple>
#include"Device.h"
#include"Surface.h"
#include"Image.h"
#include"ImageView.h"

namespace Engine{
  class Swapchain2{
    struct Chain{
      VkSwapchainKHR mHandle=nullptr;
      VkColorSpaceKHR mColorspace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      VkExtent2D mResolution={0,0};
      VkFormat mFormat=VK_FORMAT_UNDEFINED;
      uint32_t mImageCount=0;

      std::vector<SwapchainImagePtr> mImages;
      std::vector<ImageViewPtr> mImageViews;
      std::vector<VkFence> mFences;
    };


    DevicePtr mDevice=nullptr;
    SurfacePtr mSurface=nullptr;

    std::array<Chain,2> mChains;

    uint32_t mCurrentChain=0;
    uint32_t mRequestedImageCount=0;

  protected:
    //ImageCount will always be clamped to the supported min and max image count
    Swapchain2(DevicePtr &device,SurfacePtr &surface,uint32_t imageCount);

    void CreateSwapchain(uint32_t swapIndex,VkSwapchainKHR oldSwapchain);
    void GetSwapchainImages(uint32_t swapIndex);
    void CreateFences(uint32_t swapIndex);

    void DestroyFences(uint32_t swapIndex);
    void CleanupRetired();

    VkFence GetNextFence(uint32_t swapIndex);
    bool OutstandingFences(uint32_t swapIndex);

  public:
    static auto Create(DevicePtr &device,SurfacePtr surface,uint32_t imageCount=0){
      auto p=new Swapchain2(device,surface,imageCount);
      return std::shared_ptr<Swapchain2>(p);
    }

    ~Swapchain2();

    constexpr size_t size()const noexcept{
      return mChains[mCurrentChain].mImages.size();
    }

    constexpr SwapchainImagePtr &operator[](size_t index){
      return mChains[mCurrentChain].mImages[index];
    }

    std::tuple<SwapchainImagePtr,ImageViewPtr> Next();
    void PresentImage(QueuePtr &queue,SwapchainImagePtr &image);
  };

  using Swapchain2Ptr=std::shared_ptr<Swapchain2>;

}