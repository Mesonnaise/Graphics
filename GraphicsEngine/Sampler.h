#pragma once
#include<memory>
#include<vulkan/vulkan.h>

#include"Device.h"
namespace Engine{
  class Sampler{
    DevicePtr mDevice;
    VkSampler mHandle;

  protected:
    Sampler(DevicePtr &device);
  public:
    static auto Create(DevicePtr &device){
      auto p=new Sampler(device);
      return std::shared_ptr<Sampler>(p);
    }
    ~Sampler();

    constexpr VkSampler Handle(){
      return mHandle;
    }
  };

  using SamplerPtr=std::shared_ptr<Sampler>;
}
