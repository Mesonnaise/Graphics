#include <stdexcept>
#include "ImageView.h"

static PFN_vkGetDescriptorEXT pfnGetDescriptorEXT=nullptr;
static void LoadExtensions(VkDevice device){
  if(pfnGetDescriptorEXT==nullptr){
    pfnGetDescriptorEXT=reinterpret_cast<PFN_vkGetDescriptorEXT>(
      vkGetDeviceProcAddr(device,"vkGetDescriptorEXT"));
  }
}

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


ImageView::ImageView(DevicePtr &device,BaseImagePtr image):ImageView(device,image,MapImageType(image),image->Format()){}

ImageView::ImageView(DevicePtr &device,BaseImagePtr image,VkImageViewType viewType,VkFormat format):mDevice(device),mImage(image){
  LoadExtensions(mDevice->Handle());

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
      .aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
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

  switch(descriptorType){
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      descriptorSize=properties.sampledImageDescriptorSize;
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