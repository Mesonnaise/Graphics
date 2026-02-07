#pragma once
#include<memory>
#include<functional>
#include"Device.h"
namespace Engine{
  class Worker:public std::enable_shared_from_this<Worker>{
  public:
    using SetupFn=std::function<void(std::shared_ptr<Worker>)>;
    using StepFn=std::function<bool(std::shared_ptr<Worker>)>;
  
  protected:
    Worker(SetupFn setup,StepFn step);
  public:
    static inline auto Create(SetupFn setup,StepFn step) {
      auto p=new Worker(setup,step);
      return std::shared_ptr<Worker>(p);
    }
  
  };

}