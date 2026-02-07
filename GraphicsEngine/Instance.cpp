#include<stdexcept>
#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include<GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>

#include "Instance.h"
#include "Device.h"
#include"Window.h"
#undef CreateWindow

static std::atomic<uint64_t> GLFWInitCount=0;

static void InitGLFW(){
  uint64_t zeroValue=0;
  uint64_t nextValue=1;
  if(GLFWInitCount.compare_exchange_strong(zeroValue,nextValue)){
    glfwInit();
  } else
    GLFWInitCount++;
}

static void TerminateGLFW(){
  uint64_t zeroValue=0;
  uint64_t nextValue=1;
  if(GLFWInitCount.compare_exchange_strong(nextValue,zeroValue)){
    glfwTerminate();
  } else
    GLFWInitCount--;
}

static std::vector<const char *> GetGLFWExtensions(){
  uint32_t glfwExtensionCount=0;
  const char **glfwExtensionsBuffer=glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char *> glfwExtensions(glfwExtensionsBuffer,glfwExtensionsBuffer+glfwExtensionCount);
  return glfwExtensions;
}

namespace Engine{
  Instance::Physical::Physical(VkPhysicalDevice phy):mHandle(phy){}

  bool Instance::Physical::IsSurfaceSupported(int32_t queueIndex,std::shared_ptr<Window> &window)const{
    uint32_t v;
    vkGetPhysicalDeviceSurfaceSupportKHR(mHandle,queueIndex,window->Surface(),&v);

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


  VkSurfaceCapabilitiesKHR Instance::Physical::SurfaceCapabilities(std::shared_ptr<Window> &surface)const{
    VkSurfaceCapabilitiesKHR ret;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mHandle,surface->Surface(),&ret);
    return ret;
  }

  std::vector<VkSurfaceFormatKHR> Instance::Physical::SurfaceFormat(std::shared_ptr<Window> &surface)const{
    std::vector<VkSurfaceFormatKHR> ret;
    uint32_t count=0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mHandle,surface->Surface(),&count,nullptr);
    ret.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mHandle,surface->Surface(),&count,ret.data());



    return ret;
  }


  std::vector<VkPresentModeKHR> Instance::Physical::SurfacePresentModes(std::shared_ptr<Window> &window)const{
    std::vector<VkPresentModeKHR> ret;
    uint32_t count=0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(mHandle,window->Surface(),&count,nullptr);
    ret.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(mHandle,window->Surface(),&count,ret.data());
    return ret;
  }
  void Instance::Physical::DeviceImageFormatProperties(VkFormat format,VkImageType type){
    //vkGetPhysicalDeviceImageFormatProperties(mHandle,format,type,)
  }

  VkPhysicalDeviceMemoryProperties Instance::Physical::MemoryProperties(){
    VkPhysicalDeviceMemoryProperties2 properties={
      .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
      .pNext=nullptr
    };
    vkGetPhysicalDeviceMemoryProperties2(mHandle,&properties);
    return properties.memoryProperties;
  }

  Instance::Instance(std::vector<const char *> Extensions,bool Validate,bool UseGLFW):mUseGLFW(UseGLFW){
    std::vector<const char *> Layers;
    if(mUseGLFW){
      InitGLFW();
      auto glfwExtensions=GetGLFWExtensions();
      Extensions.insert(Extensions.end(),glfwExtensions.begin(),glfwExtensions.end());
    }

    if(Validate)
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

    if(mUseGLFW)
      TerminateGLFW();
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

  WindowPtr Instance::CreateWindow(int width,int height,std::string title){
    return Window::Create(shared_from_this(),width,height,title);
  }
}