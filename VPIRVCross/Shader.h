#pragma once
#include<memory>
#include<vector>
#include<filesystem>
#include<vulkan/vulkan.h>
#include<spirv_cross/spirv_cross.hpp>
class Device;
using DevicePtr=std::shared_ptr<Device>;


class Shader{
public:
  using PathGroup=std::vector<std::filesystem::path>;

private:
 


  DevicePtr mDevice=nullptr;
  VkShaderEXT mHandle=nullptr;
  bool mIsGraphic=false;

protected:
  Shader(DevicePtr &Device,PathGroup Group);

public:
  static inline std::shared_ptr<Shader> Create(DevicePtr &Device,PathGroup Group){
    auto p=new Shader(Device,Group);
    return std::shared_ptr<Shader>(p);
  }

  ~Shader();

};

