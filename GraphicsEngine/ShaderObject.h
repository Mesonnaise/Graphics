#pragma once
#include<map>
#include<set>
#include<vector>
#include<string>
#include<optional>
#include<filesystem>
#include<vulkan/vulkan.h>


#include"Device.h"
#include"DescriptorSetLayout.h"

namespace Engine{
  class ShaderObject{
  private:
    struct BindingIndices{
      VkShaderStageFlags Stages=0;
      size_t LayoutOffset=0;
      uint32_t SetIndex=0;
      size_t StructureSize=0;
      VkDescriptorSetLayoutBinding VulkanBind;
    };

    DevicePtr mDevice=nullptr;
    //PipelineLayoutPtr mPipelineLayout=nullptr;

    //The binding order needs to be maintained to allow mapping the binding to the total offset
    VkPipelineBindPoint mPipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;//This stays
    std::multimap<VkShaderStageFlags,DescriptorSetLayoutPtr> mStageLayouts;
    std::vector<DescriptorSetLayoutPtr> mDescriptorSetLayouts;
    std::vector<VkShaderEXT> mShaders;                    //This stays
    std::set<VkShaderStageFlagBits> mStages;
    std::map<std::string,BindingIndices> mBindingIndices; //This stays

    std::optional<std::tuple<std::string,uint32_t>> mPushConstant;
  protected:
    ShaderObject(DevicePtr device,std::vector<std::filesystem::path> shaderPaths,bool useBuffer);

    void AllocateSetLayouts(bool useBuffer);
    std::vector<VkDescriptorSetLayout> FindLayoutsForStage(VkShaderStageFlags stage);
  public:
    static std::shared_ptr<ShaderObject> Create(DevicePtr device,std::vector<std::filesystem::path> paths,bool useBuffer){
      auto p=new ShaderObject(device,paths,useBuffer);
      return std::shared_ptr<ShaderObject>(p);
    }

    static std::string StagesToString(VkShaderStageFlags stages);

    ~ShaderObject();

    inline std::vector<DescriptorSetLayoutPtr> GetLayouts()const{
      return mDescriptorSetLayouts;
    }

    inline std::set<VkShaderStageFlagBits> Stages()const{
      return mStages;
    }

    constexpr VkPipelineBindPoint PipelineBindPoint()const{
      return mPipelineBindPoint;
    }

    constexpr bool HashPushConstant()const{
      return mPushConstant.has_value();
    }

    constexpr std::string PushConstantName()const{
      return std::get<0>(mPushConstant.value());
    }

    constexpr uint32_t PushConstantSize()const{
      return std::get<1>(mPushConstant.value());
    }

    std::vector<std::string> GetVariableNames()const;
    VkShaderStageFlags VariableStages(std::string name)const;
    uint32_t VariableSetIndex(std::string name)const;
    VkDescriptorSetLayoutBinding VariableBinding(std::string name)const;
    VkDeviceSize VariableLayoutOffset(std::string name);
    VkDeviceSize VariableDescriptorSize(std::string name);
    size_t VariableStructureSize(std::string name);

    VkDeviceSize GetDescriptorBufferTotalSize();

    void BindShaders(VkCommandBuffer CMDBuffer);
    std::string DumpInfo();
  };

  using ShaderObjectPtr=std::shared_ptr<ShaderObject>;
}