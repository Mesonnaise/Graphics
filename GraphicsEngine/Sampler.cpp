#include<stdexcept>
#include "Sampler.h"


namespace Engine{
  Sampler::Sampler(DevicePtr &device):mDevice(device){
    VkSamplerCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext=nullptr,
      .flags=0,
      .magFilter=VK_FILTER_LINEAR,
      .minFilter=VK_FILTER_LINEAR,
      .mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU=VK_SAMPLER_ADDRESS_MODE_REPEAT ,
      .addressModeV=VK_SAMPLER_ADDRESS_MODE_REPEAT ,
      .addressModeW=VK_SAMPLER_ADDRESS_MODE_REPEAT ,
      .mipLodBias=0.0f,
      .anisotropyEnable=VK_TRUE,
      .maxAnisotropy=16.0f,
      .compareEnable=VK_FALSE,
      .compareOp=VK_COMPARE_OP_ALWAYS,
      .minLod=0.0f,
      .maxLod=VK_LOD_CLAMP_NONE,
      .borderColor=VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates=VK_FALSE
    };

    auto result=vkCreateSampler(mDevice->Handle(),&info,nullptr,&mHandle);
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to create sampler");
  }

  Sampler::~Sampler(){
    vkDestroySampler(mDevice->Handle(),mHandle,nullptr);
    mHandle=VK_NULL_HANDLE;
  }

}