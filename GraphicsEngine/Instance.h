#pragma once
#include<vector>
#include<memory>
#include<string>
#include<vulkan/vulkan.h>




#undef CreateWindow

namespace Engine{
  class Device;
  class Window;

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

      bool IsSurfaceSupported(int32_t queueIndex,std::shared_ptr<Window> &window)const;
      VkSurfaceCapabilitiesKHR SurfaceCapabilities(std::shared_ptr<Window> &window)const;
      std::vector<VkSurfaceFormatKHR> SurfaceFormat(std::shared_ptr<Window> &window)const;
      std::vector<VkPresentModeKHR> SurfacePresentModes(std::shared_ptr<Window> &window)const;
      std::vector<VkQueueFamilyProperties> QueueFamilyProperties();
      void DeviceImageFormatProperties(VkFormat format,VkImageType type);
      VkPhysicalDeviceMemoryProperties MemoryProperties();
    };
  private:
    bool mUseGLFW=false;
    VkInstance mHandle=nullptr;
  protected:
    Instance(std::vector<const char *> Extensions,bool Validate,bool UseGLFW);

  public:

    static auto Create(std::vector<const char *> Extensions,bool Validate=false,bool UseGLFW=false){
      auto n=new Instance(Extensions,Validate,UseGLFW);
      return std::shared_ptr<Instance>(n);
    }

    ~Instance();
    constexpr VkInstance Handle()const{
      return mHandle;
    }

    std::vector<Instance::Physical> EnumeratePhysicals();

    std::shared_ptr<Window> CreateWindow(int width,int height,std::string title="");

    std::shared_ptr<Device> CreateDevice(Physical phy);
    //std::shared_ptr<Device> CreateDevice(Physical phy,std::shared_ptr<Surface> surface);
    //std::shared_ptr<Surface> CreateSurface(GLFWwindow *window);
  };

  using InstancePtr=std::shared_ptr<Instance>;
}