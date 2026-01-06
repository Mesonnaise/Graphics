#include<cinttypes>
#include<fstream>
#include<stdexcept>
#include<iterator>
#include<ranges>
#include<algorithm>
#include<set>
#include<vulkan/vulkan.h>
#include"Common.h"
#include "ShaderObject.h"
#include"Reflection.h"

#include"Internal/VulkanFunctions.h"

namespace Engine{

  struct stageData{
    std::vector<uint32_t> ByteCode;
    VkShaderStageFlagBits Stage;
    std::string EntryName;
  };

  bool operator==(VkDescriptorSetLayoutBinding &l,VkDescriptorSetLayoutBinding &r){
    return l.binding==r.binding&&
      l.descriptorCount==r.descriptorCount&&
      l.descriptorType==r.descriptorType;
  }

  static std::vector<uint32_t> LoadByteCode(std::filesystem::path &path){
    std::ifstream file(path,std::ios::binary);
    std::vector<uint32_t> buffer{};

    if(!file)
      throw std::runtime_error(std::format("Unable to open file {}\n",path.string()));

    file.seekg(0,file.end);
    size_t fileSize=file.tellg();
    file.seekg(0,file.beg);

    buffer.resize(fileSize/sizeof(uint32_t));
    file.read(reinterpret_cast<char *>(buffer.data()),fileSize);

    return buffer;
  }

  std::string ShaderObject::StagesToString(VkShaderStageFlags stages){
    std::string ret;
    if(stages&VK_SHADER_STAGE_VERTEX_BIT)
      ret+="Vertex ";
    if(stages&VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
      ret+="Tessellation Control ";
    if(stages&VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
      ret+="Tessellation Evaluation ";
    if(stages&VK_SHADER_STAGE_GEOMETRY_BIT)
      ret+="Geometry ";
    if(stages&VK_SHADER_STAGE_FRAGMENT_BIT)
      ret+="Fragment ";
    if(stages&VK_SHADER_STAGE_COMPUTE_BIT)
      ret+="Compute ";
    return ret;
  }


  ShaderObject::ShaderObject(DevicePtr device,std::vector<std::filesystem::path> shaderPaths):mDevice(device){
    VkPhysicalDeviceMemoryProperties2 memoryProperties2={
      .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
      .pNext=nullptr
    };

    std::map<VkShaderStageFlagBits,stageData> stageByteCode;

    for(auto &shaderPath:shaderPaths){
      auto byteCode=LoadByteCode(shaderPath);
      auto reflection=Reflection::Create(byteCode);
      VkShaderStageFlagBits stage=reflection->Stage();
      std::set<uint32_t> SetIndices;


      stageByteCode.insert({
        stage,
        {.ByteCode=byteCode,.Stage=stage,.EntryName=reflection->EntryName()}
        });

      if(mStages.contains(reflection->Stage()))
        throw std::runtime_error("Only one shader per stage");
      mStages.insert(reflection->Stage());

      for(auto &reflectionBinding:reflection->DescriptorBindings()){
        SetIndices.insert(reflectionBinding.SetIndex);

        BindingIndices indices;
        indices.Stages=reflection->Stage();
        indices.SetIndex=reflectionBinding.SetIndex;
        indices.VulkanBind=reflectionBinding.Binding;
        indices.StructureSize=reflectionBinding.Size;

        if(mBindingIndices.contains(reflectionBinding.Name)){
          if(mBindingIndices[reflectionBinding.Name].SetIndex!=reflectionBinding.SetIndex||
            mBindingIndices[reflectionBinding.Name].VulkanBind!=reflectionBinding.Binding)
            throw std::runtime_error("All Bindings with the same name must have the same set index and binding values");

          mBindingIndices[reflectionBinding.Name].Stages|=reflection->Stage();
          mBindingIndices[reflectionBinding.Name].VulkanBind.stageFlags|=reflection->Stage();
        } else
          mBindingIndices.insert({reflectionBinding.Name,indices});
      }

      if(SetIndices.begin()!=SetIndices.end()){
        if(*SetIndices.begin()!=0)
          throw std::runtime_error("Set indices must start at 0");

        for(auto setIt=SetIndices.begin();std::next(setIt)!=SetIndices.end();setIt++){
          if((*setIt)+1!=*std::next(setIt))
            throw std::runtime_error("Set indices must be sequential");
        }
      }

      if(reflection->HasPush()){
        if(!mPushConstant.has_value())
          mPushConstant=reflection->Push();
        else
          throw std::runtime_error("Only one push constant is supported");
      }
    }

    if(mStages.contains(VK_SHADER_STAGE_COMPUTE_BIT)){
      if(mStages.size()>1)
        throw std::runtime_error("Compute shaders can't be grouped with other shaders in the same pipeline");
      mPipelineBindPoint=VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    AllocateSetLayouts();

    if(mPipelineBindPoint==VK_PIPELINE_BIND_POINT_GRAPHICS){
      VkPipelineStageFlags allStages=0;
      for(auto stage:mStages)
        allStages|=stage;

      for(auto &[stages,layout]:mStageLayouts){
        if((stages&allStages)!=allStages)
          throw std::runtime_error("All stages must have identical descriptor set layouts");
      }
    }

    std::vector<VkPushConstantRange> pushRanges;
    if(HashPushConstant()){
      VkShaderStageFlags stages=0;
      for(auto stage:Stages())
        stages|=stage;

      VkPushConstantRange range{
        .stageFlags=stages,
        .offset=0,
        .size=PushConstantSize()
      };
      pushRanges.push_back(range);
    }

    std::vector<std::vector<VkDescriptorSetLayout>> layoutHandles;

    std::vector<VkShaderCreateInfoEXT> createInfo;
    for(auto it=stageByteCode.begin();it!=stageByteCode.end();it++){
      VkShaderStageFlags nextStage=0;



      auto nextIt=std::next(it);
      if(nextIt!=stageByteCode.end())
        nextStage=nextIt->first;

      auto &shaderData=it->second;
      layoutHandles.push_back(FindLayoutsForStage(it->first));

      VkShaderCreateInfoEXT info={
        .sType=VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
        .pNext=nullptr,
        //This is causing an access violation in the amdkv64.dll library 
        .flags=0,
        .stage=(VkShaderStageFlagBits)it->first,
        .nextStage=nextStage,
        .codeType=VK_SHADER_CODE_TYPE_SPIRV_EXT,
        .codeSize=(uint32_t)shaderData.ByteCode.size()*sizeof(uint32_t),
        .pCode=shaderData.ByteCode.data(),
        .pName=shaderData.EntryName.c_str(),
        .setLayoutCount=(uint32_t)layoutHandles.back().size(),
        .pSetLayouts=layoutHandles.back().data(),
        .pushConstantRangeCount=(uint32_t)pushRanges.size(),
        .pPushConstantRanges=pushRanges.data(),
        .pSpecializationInfo=nullptr
      };
      createInfo.push_back(info);
    }

    mShaders.resize(createInfo.size(),nullptr);

    auto result=pfCreateShader(mDevice->Handle(),(uint32_t)createInfo.size(),createInfo.data(),nullptr,mShaders.data());
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to create shaders");
  }

  ShaderObject::~ShaderObject(){
    for(auto shader:mShaders)
      pfDestroyShader(mDevice->Handle(),shader,nullptr);
  }

  void ShaderObject::AllocateSetLayouts(){
    std::map<VkShaderStageFlags,std::map<uint32_t,std::vector<BindingIndices *>>> mapping;

    for(auto &[name,bindingIndice]:mBindingIndices){
      mapping[bindingIndice.Stages][bindingIndice.SetIndex].push_back(&bindingIndice);
    }
    for(auto &[stages,sets]:mapping){
      for(auto &[setIndex,bindings]:sets){
        std::vector<VkDescriptorSetLayoutBinding> vkBindings;
        for(auto &bindings:bindings){
          bindings->LayoutOffset=mDescriptorSetLayouts.size();
          vkBindings.push_back(bindings->VulkanBind);
        }
        auto setLayout=DescriptorSetLayout::Create(mDevice,vkBindings);

        mDescriptorSetLayouts.push_back(setLayout);
        mStageLayouts.insert({stages,setLayout});
      }
    }
  }

  std::vector<VkDescriptorSetLayout> ShaderObject::FindLayoutsForStage(VkShaderStageFlags stage){
    std::vector<VkDescriptorSetLayout> ret;
    for(auto &[layoutStage,layout]:mStageLayouts){
      if(layoutStage&stage)
        ret.push_back(layout->Handle());
    }
    return ret;
  };

  void ShaderObject::BindShaders(VkCommandBuffer CMDBuffer){
    std::vector<VkShaderStageFlagBits> stages(mStages.begin(),mStages.end());

    pfCmdBindShaders(CMDBuffer,(uint32_t)mShaders.size(),stages.data(),mShaders.data());
   // pfCmdSetDescriptorBufferOffsetsEXT(CMDBuffer,mBindPoint,mPipelineLayout,)
  }

  uint32_t ShaderObject::VariableSetIndex(std::string name)const{
    if(mBindingIndices.contains(name)){
      return mBindingIndices.at(name).SetIndex;
    } else
      throw std::runtime_error("No binding with name found");
  }

  VkShaderStageFlags ShaderObject::VariableStages(std::string name)const{
    if(mBindingIndices.contains(name)){
      return mBindingIndices.at(name).Stages;
    } else
      throw std::runtime_error("No binding with name found");
  }

  VkDescriptorSetLayoutBinding ShaderObject::VariableBinding(std::string name)const{
    if(mBindingIndices.contains(name)){
      return mBindingIndices.at(name).VulkanBind;
    } else
      throw std::runtime_error("No binding with name found");
  }

  VkDeviceSize ShaderObject::VariableLayoutOffset(std::string name){
    VkDeviceSize globalOffset=0;
    VkDeviceSize bindingOffset=0;

    if(mBindingIndices.contains(name)){
      auto &binding=mBindingIndices.at(name);
      for(size_t layoutIndex=0;layoutIndex<binding.LayoutOffset;layoutIndex++){
        VkDeviceSize layoutSize=0;
        pfGetDescriptorSetLayoutSize(
          mDevice->Handle(),
          mDescriptorSetLayouts[layoutIndex]->Handle(),
          &layoutSize);
        globalOffset+=layoutSize;
      }

      pfGetDescriptorSetLayoutBindingOffsetEXT(
        mDevice->Handle(),
        mDescriptorSetLayouts[binding.LayoutOffset]->Handle(),
        binding.VulkanBind.binding,
        &bindingOffset);

      return globalOffset+bindingOffset;
    } else
      throw std::runtime_error("No binding with name found");
  }

  size_t ShaderObject::VariableStructureSize(std::string name){
    if(mBindingIndices.contains(name)){
      auto &binding=mBindingIndices.at(name);
      switch(binding.VulkanBind.descriptorType){
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
          return binding.StructureSize;
        default:
          throw std::runtime_error("Descriptor type not supported for structure size retrieval");
      }
    } else
      throw std::runtime_error("No binding with name found");
  }

  VkDeviceSize ShaderObject::VariableDescriptorSize(std::string name){
    if(mBindingIndices.contains(name)){
      const auto descriptorBufferProperties=mDevice->GetDescriptorProperties();
      auto &binding=mBindingIndices.at(name);

      //Add robust buffer types if needed
      switch(binding.VulkanBind.descriptorType){
        case VK_DESCRIPTOR_TYPE_SAMPLER:
          return descriptorBufferProperties.samplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
          return descriptorBufferProperties.combinedImageSamplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
          return descriptorBufferProperties.sampledImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
          return descriptorBufferProperties.storageImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
          return descriptorBufferProperties.uniformTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
          return descriptorBufferProperties.storageTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
          return descriptorBufferProperties.uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
          return descriptorBufferProperties.storageBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
          return descriptorBufferProperties.inputAttachmentDescriptorSize;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
          return descriptorBufferProperties.accelerationStructureDescriptorSize;
        default:
          throw std::runtime_error("Descriptor type not supported for size retrieval");
      }
    } else
      throw std::runtime_error("No binding with name found");
  }

  VkDeviceSize ShaderObject::GetDescriptorBufferTotalSize(){
    VkDeviceSize total=0;
    for(auto &setLayout:mDescriptorSetLayouts){
      VkDeviceSize layoutSize=0;
      pfGetDescriptorSetLayoutSize(mDevice->Handle(),setLayout->Handle(),&layoutSize);
      total+=layoutSize;
    }
    return total;
  }

  std::vector<std::string> ShaderObject::GetVariableNames()const{
    std::vector<std::string> ret;
    std::ranges::copy(mBindingIndices|std::views::keys,std::back_inserter(ret));
    return ret;
  }

  std::string ShaderObject::DumpInfo(){
    std::string ret;
    for(auto &name:GetVariableNames()){
      auto binding=VariableBinding(name);
      auto stages=StagesToString(VariableStages(name));
      auto setIndex=VariableSetIndex(name);
      auto offset=VariableLayoutOffset(name);
      auto size=VariableDescriptorSize(name);
      ret+=std::format("Variable {} Stages {} binding {}:{} offset {} size {}\n",
        name,stages,setIndex,binding.binding,offset,size);
    }
    return ret;
  }
}