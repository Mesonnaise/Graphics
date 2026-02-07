#pragma once
#include<memory>
#include<vulkan/vulkan.h>
#include<GLFW/glfw3.h>

//#include"Instance.h"

namespace Engine{
  class Instance;

  class Surface:public std::enable_shared_from_this<Surface>{
    VkSurfaceKHR mHandle;
    std::shared_ptr<Instance> mInstance;

  protected:
    Surface(std::shared_ptr<Instance> ins,GLFWwindow *window);
  public:
    static auto Create(std::shared_ptr<Instance> ins,GLFWwindow *window){
      auto p=new Surface(ins,window);
      return std::shared_ptr<Surface>(p);
    }

    ~Surface();

    constexpr VkSurfaceKHR Handle()const{
      return mHandle;
    }
  };

  using SurfacePtr=std::shared_ptr<Surface>;
}

