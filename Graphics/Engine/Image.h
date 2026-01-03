#pragma once
#include<memory>
#include<vulkan/vulkan.h>
#include "Resource.h"

class ImageView;

class BaseImage:public Resource{
protected:
  VkImageLayout mCurrentLayout=VK_IMAGE_LAYOUT_UNDEFINED;
  VkExtent3D mExtent={1,1,1};
  VkImageType mType=VK_IMAGE_TYPE_1D;
  VkFormat mFormat=VK_FORMAT_UNDEFINED;
  VkImageUsageFlags mUsage=VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;

  BaseImage(DevicePtr &device);
public:

  constexpr VkImage Handle(){
    return mImage;
  }

  constexpr VkImageLayout GetLayout(){
    return mCurrentLayout;
  }

  constexpr void SetLayout(VkImageLayout layout){
    mCurrentLayout=layout;
  }

  constexpr VkImageType ImageType(){
    return mType;
  }

  constexpr VkFormat Format(){
    return mFormat;
  }

  constexpr VkExtent3D Extent(){
    return mExtent;
  }

  constexpr VkImageUsageFlags Usage(){
    return mUsage;
  }

  VkMemoryRequirements MemoryRequirements() override;
  std::shared_ptr<ImageView> CreateImageView();
};

class Image:public BaseImage{

protected:
  VkImageLayout mCurrentLayout=VK_IMAGE_LAYOUT_UNDEFINED;

  Image(DevicePtr &device,VkExtent3D extent,VkFormat format,VkImageUsageFlags usage);
public:
  static std::shared_ptr<Image> Create(DevicePtr &device,VkExtent3D extent,VkFormat format,VkImageUsageFlags usage){
    auto p=new Image(device,extent,format,usage);
    return std::shared_ptr<Image>(p);
  }

  ~Image() override;

  constexpr VkImage Handle(){
    return mImage;
  }
};

class SwapchainImage:public BaseImage{
  friend class Swapchain;

  VkImageLayout mCurrentLayout=VK_IMAGE_LAYOUT_UNDEFINED;

protected:
  SwapchainImage(DevicePtr &device,VkImage image,VkImageType type,VkFormat format,VkExtent3D extent);

  static std::shared_ptr<SwapchainImage> Create(DevicePtr &device,VkImage image,VkImageType type,VkFormat format,VkExtent3D extent){
    auto p=new SwapchainImage(device,image,type,format,extent);
    return std::shared_ptr<SwapchainImage>(p);
  }
public:

  ~SwapchainImage() override;

  constexpr VkImage Handle(){
    return mImage;
  }
};

using BaseImagePtr=std::shared_ptr<BaseImage>;
using ImagePtr=std::shared_ptr<Image>;
using SwapchainImagePtr=std::shared_ptr<SwapchainImage>;
