#pragma once
#include<memory>
#include<optional>
#include<string>
#include<vulkan/vulkan.h>

#include"Instance.h"
#include"Surface.h"

namespace Engine{
  class Swapchain;

  class Device:public std::enable_shared_from_this<Device>{
    friend class Instance;
  public:
    class Queue{
      friend class Device;

      std::shared_ptr<Device> mDevice=nullptr;
      VkQueue mHandle=nullptr;
      uint32_t mQueueFamily=0;
      uint32_t mQueueIndex=0;

    protected:
      Queue(std::shared_ptr<Device> device,uint32_t QueueFamily,uint32_t QueueIndex);

    public:
      constexpr VkQueue Handle(){
        return mHandle;
      }

      constexpr uint32_t QueueFamily()const{
        return mQueueFamily;
      }

      constexpr uint32_t QueueIndex()const{
        return mQueueIndex;
      }
      void Submit(std::vector<VkCommandBuffer> CommandBuffers);

      void Wait();
    };

  private:
    InstancePtr mInstance;
    Instance::Physical mPhysical;

    VkDevice mHandle=nullptr;
    uint32_t mPrimaryQueueFamily;



    std::optional<uint32_t> mComputeQueueFamily;
    std::optional<uint32_t> mTransferQueueFamily;

  protected:

    Device(InstancePtr ins,Instance::Physical phy,std::optional<SurfacePtr> surface);

    static auto Create(InstancePtr ins,Instance::Physical phy,std::optional<SurfacePtr> surface){
      auto p=new Device(ins,phy,surface);
      return std::shared_ptr<Device>(p);
    }

  public:
    ~Device();

    inline auto GetQueue(uint32_t Index){
      auto p=new Queue(shared_from_this(),mPrimaryQueueFamily,Index);
      return std::shared_ptr<Device::Queue>(p);
    }

    constexpr uint32_t GetPrimiaryQueueFamily(){
      return mPrimaryQueueFamily;
    }

    constexpr VkDevice Handle()const{
      return mHandle;
    }

    inline Instance::Physical Physical()const{
      return mPhysical;
    }

    VkPhysicalDeviceDescriptorBufferPropertiesEXT GetDescriptorProperties()const;


    std::shared_ptr<Swapchain> CreateSwapchain(SurfacePtr surface);

    void Debug(VkObjectType type,uint64_t handle,std::string name);
  };

  using DevicePtr=std::shared_ptr<Device>;
  using QueuePtr=std::shared_ptr<Device::Queue>;
}
