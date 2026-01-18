#include <stdexcept>
#include "ImageView.h"

#include"Internal/VulkanFunctions.h"

namespace Engine{

  static VkImageViewType MapImageType(BaseImagePtr &image){
    switch(image->ImageType()){
      case VK_IMAGE_TYPE_1D:
        return VK_IMAGE_VIEW_TYPE_1D;
      case VK_IMAGE_TYPE_2D:
        return VK_IMAGE_VIEW_TYPE_2D;
      case VK_IMAGE_TYPE_3D:
        return VK_IMAGE_VIEW_TYPE_3D;
      default:
        return VK_IMAGE_VIEW_TYPE_2D;
    }
  }


  ImageView::ImageView(DevicePtr &device,BaseImagePtr image):
    ImageView(device,image,MapImageType(image),image->Format()){}

  ImageView::ImageView(DevicePtr &device,BaseImagePtr image,VkImageViewType viewType,VkFormat format):
    mDevice(device),mImage(image){
    VkImageAspectFlags aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;

    if(format==VK_FORMAT_D32_SFLOAT)
      aspectMask=VK_IMAGE_ASPECT_STENCIL_BIT;
    else if(format==VK_FORMAT_S8_UINT)
      aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
    else if(format==VK_FORMAT_D16_UNORM_S8_UINT||format==VK_FORMAT_D24_UNORM_S8_UINT||format==VK_FORMAT_D32_SFLOAT_S8_UINT)
      aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT;

    VkImageViewCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext=nullptr,
      .flags=0,
      .image=mImage->Handle(),
      .viewType=viewType,
      .format=format,
      .components={
        .r=VK_COMPONENT_SWIZZLE_IDENTITY,
        .g=VK_COMPONENT_SWIZZLE_IDENTITY,
        .b=VK_COMPONENT_SWIZZLE_IDENTITY,
        .a=VK_COMPONENT_SWIZZLE_IDENTITY
      },
      .subresourceRange={
        .aspectMask=aspectMask,
        .baseMipLevel=0,
        .levelCount=1,
        .baseArrayLayer=0,
        .layerCount=1
      }
    };
    auto status=vkCreateImageView(mDevice->Handle(),&info,nullptr,&mHandle);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to create image view");
  }

  ImageView::ImageView(
    DevicePtr &device,BaseImagePtr image,VkImageViewType viewType,
    VkFormat format,VkImageAspectFlags aspect):
    mDevice(device),mImage(image){

    VkImageViewCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext=nullptr,
      .flags=0,
      .image=mImage->Handle(),
      .viewType=viewType,
      .format=format,
      .components={
        .r=VK_COMPONENT_SWIZZLE_IDENTITY,
        .g=VK_COMPONENT_SWIZZLE_IDENTITY,
        .b=VK_COMPONENT_SWIZZLE_IDENTITY,
        .a=VK_COMPONENT_SWIZZLE_IDENTITY
      },
      .subresourceRange={
        .aspectMask=aspect,
        .baseMipLevel=0,
        .levelCount=1,
        .baseArrayLayer=0,
        .layerCount=1
      }
    };
    auto status=vkCreateImageView(mDevice->Handle(),&info,nullptr,&mHandle);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Unable to create image view");
  }

  ImageView::~ImageView(){
    vkDestroyImageView(mDevice->Handle(),mHandle,nullptr);
  }

  size_t ImageView::GetDescriptor(void *WritePointer,VkDescriptorType descriptorType,VkImageLayout layout){
    auto properties=mDevice->GetDescriptorProperties();
    size_t descriptorSize=0;

    auto CreateSampler=[&](){
      if(mSampler==nullptr)
        mSampler=Sampler::Create(mDevice);
    };

    switch(descriptorType){
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        descriptorSize=properties.sampledImageDescriptorSize;
        CreateSampler();
        break;
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        descriptorSize=properties.combinedImageSamplerDescriptorSize;
        CreateSampler();
        break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        descriptorSize=properties.storageImageDescriptorSize;
        break;
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        descriptorSize=properties.inputAttachmentDescriptorSize;
        break;
      default:
        throw std::runtime_error("Unsupported image descriptor type.");
    }

    VkDescriptorImageInfo imageInfo={
      .sampler=VK_NULL_HANDLE,
      .imageView=mHandle,
      .imageLayout=layout
    };

    if(mSampler)
      imageInfo.sampler=mSampler->Handle();

    VkDescriptorGetInfoEXT descriptorInfo={
      .sType=VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
      .pNext=nullptr,
      .type=descriptorType,
      .data={
        .pSampledImage=&imageInfo
      }
    };

    pfnGetDescriptorEXT(mDevice->Handle(),&descriptorInfo,descriptorSize,WritePointer);

    return descriptorSize;
  }

  VkRenderingAttachmentInfo ImageView::BasicAttachment(VkImageLayout layout){
    VkRenderingAttachmentInfo attachmentInfo={
      .sType=VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext=nullptr,
      .imageView=mHandle,
      .imageLayout=layout,
      .resolveMode=VK_RESOLVE_MODE_NONE,
      .resolveImageView=VK_NULL_HANDLE,
      .resolveImageLayout=VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp=VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue={.color={0.0,0.0,0.0,0.0}}
    };

    return attachmentInfo;
  }
}