#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3.h>
#include<GLFW/glfw3native.h>
#include "Surface.h"

Surface::Surface(InstancePtr ins,GLFWwindow *window):mInstance(ins){
  VkWin32SurfaceCreateInfoKHR info={
    .sType=VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .pNext=nullptr,
    .hinstance=GetModuleHandle(nullptr),
    .hwnd=glfwGetWin32Window(window)
  };
  vkCreateWin32SurfaceKHR(ins->Handle(),&info,nullptr,&mHandle);
}

Surface::~Surface(){
  vkDestroySurfaceKHR(mInstance->Handle(),mHandle,nullptr);
}