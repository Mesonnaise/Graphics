#include<stdexcept>
#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>
#include "Instance.h"
#include "Device.h"

namespace Engine{
  Instance::Physical::Physical(VkPhysicalDevice phy):mHandle(phy){}

  bool Instance::Physical::IsSurfaceSupported(int32_t queueIndex,std::shared_ptr<Surface> &surface)const{
    uint32_t v;
    vkGetPhysicalDeviceSurfaceSupportKHR(mHandle,queueIndex,surface->Handle(),&v);

    return (bool)v;
  }

  std::vector<VkQueueFamilyProperties> Instance::Physical::QueueFamilyProperties(){
    std::vector<VkQueueFamilyProperties> ret;
    uint32_t count=0;
    vkGetPhysicalDeviceQueueFamilyProperties(mHandle,&count,nullptr);
    ret.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(mHandle,&count,ret.data());
    return ret;
  }


  VkSurfaceCapabilitiesKHR Instance::Physical::SurfaceCapabilities(std::shared_ptr<Surface> &surface)const{
    VkSurfaceCapabilitiesKHR ret;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mHandle,surface->Handle(),&ret);
    return ret;
  }

  std::vector<VkSurfaceFormatKHR> Instance::Physical::SurfaceFormat(std::shared_ptr<Surface> &surface)const{
    std::vector<VkSurfaceFormatKHR> ret;
    uint32_t count=0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mHandle,surface->Handle(),&count,nullptr);
    ret.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mHandle,surface->Handle(),&count,ret.data());



    return ret;
  }


  std::vector<VkPresentModeKHR> Instance::Physical::SurfacePresentModes(std::shared_ptr<Surface> &surface)const{
    std::vector<VkPresentModeKHR> ret;
    uint32_t count=0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(mHandle,surface->Handle(),&count,nullptr);
    ret.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(mHandle,surface->Handle(),&count,ret.data());
    return ret;
  }
  void Instance::Physical::DeviceImageFormatProperties(VkFormat format,VkImageType type){
   // vkGetPhysicalDeviceImageFormatProperties(mHandle,foramt,type,)
  }

  VkPhysicalDeviceMemoryProperties Instance::Physical::MemoryProperties(){
    VkPhysicalDeviceMemoryProperties2 properties={
      .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
      .pNext=nullptr
    };
    vkGetPhysicalDeviceMemoryProperties2(mHandle,&properties);
    return properties.memoryProperties;
  }

  Instance::Instance(std::vector<const char *> Extensions){
    std::vector<const char *> Layers;
    //std::vector<const char *> Extensions;

    Layers.push_back("VK_LAYER_KHRONOS_validation");

    VkApplicationInfo application={
      .sType=VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName="Runtime",
      .applicationVersion=VK_MAKE_VERSION(1, 0, 0),
      .pEngineName="RuntimeEngine",
      .engineVersion=VK_MAKE_VERSION(1, 0, 0),
      .apiVersion=VK_API_VERSION_1_4
    };

    VkInstanceCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext=nullptr,
      .pApplicationInfo=&application,

      .enabledLayerCount=(uint32_t)Layers.size(),
      .ppEnabledLayerNames=Layers.data(),

      .enabledExtensionCount=(uint32_t)Extensions.size(),
      .ppEnabledExtensionNames=Extensions.data()
    };

    auto status=vkCreateInstance(&info,nullptr,&mHandle);
    if(status!=VK_SUCCESS)
      throw std::runtime_error("Failed to create Vulkan instance");
  }

  Instance::~Instance(){
    vkDestroyInstance(mHandle,nullptr);

  }

  std::vector<Instance::Physical> Instance::EnumeratePhysicals(){
    std::vector<VkPhysicalDevice> vkPhy;
    std::vector<Physical> ret;
    uint32_t deviceCount=0;

    vkEnumeratePhysicalDevices(mHandle,&deviceCount,nullptr);
    vkPhy.resize(deviceCount);
    vkEnumeratePhysicalDevices(mHandle,&deviceCount,vkPhy.data());

    for(auto p:vkPhy)
      ret.push_back(p);

    return ret;
  }

  std::shared_ptr<Device> Instance::CreateDevice(Physical phy){
    return Device::Create(shared_from_this(),phy,{});
  }
  std::shared_ptr<Device> Instance::CreateDevice(Physical phy,std::shared_ptr<Surface> surface){
    return Device::Create(shared_from_this(),phy,surface);
  }
  std::shared_ptr<Surface> Instance::CreateSurface(GLFWwindow *window){
    return Surface::Create(shared_from_this(),window);
  }
}