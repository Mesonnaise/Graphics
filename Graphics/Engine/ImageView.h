#pragma once
#include<memory>
#include<vulkan/vulkan.h>

#include"Device.h"
#include"Image.h"

class ImageView{
  DevicePtr mDevice=nullptr;
  BaseImagePtr mImage=nullptr;
  VkImageView mHandle=nullptr;

protected:
  ImageView(DevicePtr &device,BaseImagePtr image);
  ImageView(DevicePtr &device,BaseImagePtr image,VkImageViewType viewType,VkFormat format);
public:
  static std::shared_ptr<ImageView> Create(DevicePtr &device,BaseImagePtr image){
    auto p=new ImageView(device,image);
    return std::shared_ptr<ImageView>(p);
  }

  static std::shared_ptr<ImageView> Create(DevicePtr &device,BaseImagePtr image,VkImageViewType viewType,VkFormat format){
    auto p=new ImageView(device,image,viewType,format);
    return std::shared_ptr<ImageView>(p);
  }
  ~ImageView();

  constexpr VkImageView Handle(){
    return mHandle;
  }
  size_t GetDescriptor(void *WritePointer,VkDescriptorType descriptorType,VkImageLayout layout);
};

using ImageViewPtr=std::shared_ptr<ImageView>;
