#pragma once
#include<vector>
#include<memory>
#include<vulkan/vulkan.h>
#include<GLFW/glfw3.h>

namespace Engine{
  class Device;
  class Surface;

  class Instance:public std::enable_shared_from_this<Instance>{
  public:
    class Physical{
      friend class Instance;
    protected:
      VkPhysicalDevice mHandle=nullptr;
      Physical(VkPhysicalDevice);
    public:
      constexpr VkPhysicalDevice Handle()const{
        return mHandle;
      }

      bool IsSurfaceSupported(int32_t queueIndex,std::shared_ptr<Surface> &surface)const;
      VkSurfaceCapabilitiesKHR SurfaceCapabilities(std::shared_ptr<Surface> &surface)const;
      std::vector<VkSurfaceFormatKHR> SurfaceFormat(std::shared_ptr<Surface> &surface)const;
      std::vector<VkPresentModeKHR> SurfacePresentModes(std::shared_ptr<Surface> &surface)const;
      std::vector<VkQueueFamilyProperties> QueueFamilyProperties();
      void DeviceImageFormatProperties(VkFormat format,VkImageType type);
      VkPhysicalDeviceMemoryProperties MemoryProperties();
    };
  private:

    VkInstance mHandle=nullptr;
  protected:
    Instance(std::vector<const char *> Extensions);

  public:

    static auto Create(std::vector<const char *> Extensions){
      auto n=new Instance(Extensions);
      return std::shared_ptr<Instance>(n);
    }

    ~Instance();
    constexpr VkInstance Handle()const{
      return mHandle;
    }

    std::vector<Instance::Physical> EnumeratePhysicals();


    std::shared_ptr<Device> CreateDevice(Physical phy);
    std::shared_ptr<Device> CreateDevice(Physical phy,std::shared_ptr<Surface> surface);
    std::shared_ptr<Surface> CreateSurface(GLFWwindow *window);
  };

  using InstancePtr=std::shared_ptr<Instance>;
}