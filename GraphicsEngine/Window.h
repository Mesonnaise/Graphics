#pragma once
#include<memory>
#include<string>
#include<functional>
#include"Surface.h"
namespace Engine{
  class Instance;

  class Window{
  public:
    using ResizeCallbackType=std::function<void(int width,int height)>;
    using CloseCallbackType=std::function<void()>;
    using RefreshCallbackType=std::function<void()>;
    using KeyCallbackType=std::function<void(int key,int scancode,int action,int mods)>;
    using MouseButtonCallbackType=std::function<void(int button,int action,int mods)>;
    using MouseMoveCallbackType=std::function<void(double xPos,double yPos)>;
  private:
    std::shared_ptr<Instance> mInstance; 
    VkSurfaceKHR mSurface;
    void *mWindow=nullptr;
    struct FunctionPointers{
      ResizeCallbackType ResizeCallback;
      CloseCallbackType CloseCallback;
      RefreshCallbackType RefreshCallback;
      KeyCallbackType KeyCallback;
      MouseButtonCallbackType MouseButtonCallback;
      MouseMoveCallbackType MouseMoveCallback;
    } mPointers;
    

  protected:

    Window(std::shared_ptr<Instance> instance,int width,int height,std::string title);

  public:
    static inline auto Create(std::shared_ptr<Instance> instance,int width,int height,std::string title=""){
      auto p=new Window(instance,width,height,title);
      return std::unique_ptr<Window>(p);
    }

    ~Window();

    inline VkSurfaceKHR Surface(){
      return mSurface;
    }

    void ResizeCallback(ResizeCallbackType resizeFN);
    void CloseCallback(CloseCallbackType closeFN);
    void RefreshCallback(RefreshCallbackType refreshFN);
    void KeyCallback(KeyCallbackType keyFN);
    void MouseButtonCallback(MouseButtonCallbackType mouseButtonFN);
    void MouseMoveCallback(MouseMoveCallbackType mouseMoveFN);

    bool ShouldClose()const;
  };

  using WindowPtr=std::shared_ptr<Window>;
}