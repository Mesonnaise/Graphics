#pragma once
#include<vector>
#include<memory>
#include<filesystem>
#include<map>
#include<string>
#include<tuple>
#include<vulkan/vulkan.h>
#include"Engine/Common.h"
#include"Engine/Device.h"
#include"Engine/DescriptorSetLayout.h"

class ShaderGroup: public std::enable_shared_from_this<ShaderGroup>{

  struct SetMeta{
    uint32_t SetIndex=0;

    std::map<uint32_t,DescriptorLayoutBinding> Bindings;
    std::map<uint32_t,std::string> BindingNames;

    bool operator<(const SetMeta &r)const{
      if(SetIndex!=r.SetIndex) return SetIndex<r.SetIndex;
      return Bindings<r.Bindings;
    }

    bool operator==(const SetMeta&r)const{
      if(SetIndex!=r.SetIndex) return false;
      return Bindings==r.Bindings;
    }
  };

  struct ShaderMeta{
    std::map<uint32_t,SetMeta> SetBindings;
    std::map<uint32_t,DescriptorSetLayoutPtr> Layouts;
    //Byte code can be discarded after shader creation
    //Better to seperate from the retained shader meta data
    std::vector<uint32_t> ByteCode;
    std::string EntryPoint;
    //Would be better have the shader objects in a separate vector
    VkShaderEXT ShaderObject=nullptr;
    VkShaderStageFlags Stage=0;
  };

  PFN_vkCreateShadersEXT pfCreateShader=nullptr;
  PFN_vkDestroyShaderEXT pfDestroyShader=nullptr;
  PFN_vkCmdBindShadersEXT pfCmdBindShaders=nullptr;
  PFN_vkGetDescriptorSetLayoutSizeEXT pfGetDescriptorSetLayoutSize=nullptr;

  std::vector<ShaderMeta> mShaders;
  std::vector<DescriptorSetLayoutPtr> mLayouts;
  std::map<std::string,std::tuple<uint32_t,uint32_t>> mBindingNames;
  //std::map<VkShaderStageFlagBits,VkShaderEXT> mShaderObjects;
  //std::multimap<VkShaderStageFlagBits,uint32_t>
  DevicePtr mDevice=nullptr;
  bool mLink=false;
  bool mCompute=false;

protected:
  ShaderGroup(DevicePtr &Device,std::vector<std::filesystem::path> ShaderPaths);
  void LoadShaders(std::vector<std::filesystem::path> &ShaderPaths);

  void CreateShaders(std::vector<ShaderMeta> &shaders);
public:
  static auto Create(DevicePtr &Device,std::vector<std::filesystem::path> ShaderPaths){
    auto p=new ShaderGroup(Device,ShaderPaths);
    return std::shared_ptr<ShaderGroup>(p);
  }

  ~ShaderGroup();
  void BindShaders(VkCommandBuffer buffer);
  
  VkDeviceSize TotalDescriptorSize();
  

  std::vector<DescriptorSetLayoutPtr> GetLayouts();
  std::vector<std::string> GetVariableNames()const;
  VkDescriptorType GetVariableType(const std::string name)const;
};

using ShaderGroupPtr=std::shared_ptr<ShaderGroup>;