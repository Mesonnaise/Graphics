#include<stdexcept>
#include<cinttypes>
#include<atomic>
#include<GLFW/glfw3.h>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include<GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>
#include"Instance.h"
#include "Window.h"

namespace Engine{

  Window::Window(std::shared_ptr<Instance> instance,int width,int height,std::string title):mInstance(instance){
    mWindow=glfwCreateWindow(width,height,title.c_str(),nullptr,nullptr);
    if(!mWindow)
      throw std::runtime_error("Failed to create GLFW window");


    glfwSetWindowUserPointer((GLFWwindow *)mWindow,&mPointers);

    VkWin32SurfaceCreateInfoKHR info={
      .sType=VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .pNext=nullptr,
      .hinstance=GetModuleHandle(nullptr),
      .hwnd=glfwGetWin32Window((GLFWwindow*)mWindow)
    };
    vkCreateWin32SurfaceKHR(mInstance->Handle(),&info,nullptr,&mSurface);

    glfwSetWindowSizeCallback((GLFWwindow *)mWindow,[](GLFWwindow *window,int width,int height){
      auto ptr=reinterpret_cast<FunctionPointers *>(glfwGetWindowUserPointer(window));
      if(ptr->ResizeCallback)
        ptr->ResizeCallback(width,height);
    });

    glfwSetWindowCloseCallback((GLFWwindow *)mWindow,[](GLFWwindow *window){
      auto ptr=reinterpret_cast<FunctionPointers *>(glfwGetWindowUserPointer(window));
      if(ptr->CloseCallback)
        ptr->CloseCallback();
    });

    glfwSetWindowRefreshCallback((GLFWwindow *)mWindow,[](GLFWwindow *window){
      auto ptr=reinterpret_cast<FunctionPointers *>(glfwGetWindowUserPointer(window));
      if(ptr->RefreshCallback)
        ptr->RefreshCallback();
    });

    glfwSetKeyCallback((GLFWwindow *)mWindow,[](GLFWwindow *window,int key,int scancode,int action,int mods){
      auto ptr=reinterpret_cast<FunctionPointers *>(glfwGetWindowUserPointer(window));
      if(ptr->KeyCallback)
        ptr->KeyCallback(key,scancode,action,mods);
    });

    glfwSetMouseButtonCallback((GLFWwindow *)mWindow,[](GLFWwindow *window,int button,int action,int mods){
      auto ptr=reinterpret_cast<FunctionPointers *>(glfwGetWindowUserPointer(window));
      if(ptr->MouseButtonCallback)
        ptr->MouseButtonCallback(button,action,mods);
    });

    glfwSetCursorPosCallback((GLFWwindow *)mWindow,[](GLFWwindow *window,double xPos,double yPos){
      auto ptr=reinterpret_cast<FunctionPointers *>(glfwGetWindowUserPointer(window));
      if(ptr->MouseMoveCallback)
        ptr->MouseMoveCallback(xPos,yPos);
    });
  }

  Window::~Window(){
    if(mWindow)
      glfwDestroyWindow((GLFWwindow *)mWindow);

    if(mSurface)
      vkDestroySurfaceKHR(mInstance->Handle(),mSurface,nullptr);
  }
  
  void Window::ResizeCallback(ResizeCallbackType resizeFN){
    mPointers.ResizeCallback=resizeFN;
  }

  void Window::CloseCallback(CloseCallbackType closeFN){
    mPointers.CloseCallback=closeFN;
  }
  void Window::RefreshCallback(RefreshCallbackType refreshFN){
    mPointers.RefreshCallback=refreshFN;
  }
  void Window::KeyCallback(KeyCallbackType keyFN){
    mPointers.KeyCallback=keyFN;
  }
  void Window::MouseButtonCallback(MouseButtonCallbackType mouseButtonFN){
    mPointers.MouseButtonCallback=mouseButtonFN;
  }
  void Window::MouseMoveCallback(MouseMoveCallbackType mouseMoveFN){
    mPointers.MouseMoveCallback=mouseMoveFN;
  }

  bool Window::ShouldClose()const{
    return glfwWindowShouldClose((GLFWwindow *)mWindow);
  }
}

