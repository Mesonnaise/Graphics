#include<vector>
#include<fstream>
#include<set>
#include<algorithm>
#include<numeric>
#include<stdexcept>

#include"Engine/Device.h"
#include"Engine/DescriptorSetLayout.h"
#include"ShaderGroup.h"
#include"Engine/Reflection.h"
#include"Engine/Common.h"

using LayoutBindingMap=std::multimap<uint32_t,DescriptorLayoutBinding>;


static std::set<uint32_t> GetSetIndices(LayoutBindingMap &bindings){
  std::set<uint32_t> ret;
  for(auto &[setIndex,unused]:bindings)
    ret.insert(setIndex);
  return ret;
}

static std::vector<uint32_t> LoadByteCode(std::filesystem::path &path){
  std::ifstream file(path,std::ios::binary);
  std::vector<uint32_t> buffer;

  if(!file)
    throw std::runtime_error(std::format("Unable to open file {}\n",path.string()));

  file.seekg(0,file.end);
  size_t fileSize=file.tellg();
  file.seekg(0,file.beg);

  buffer.resize(fileSize/sizeof(uint32_t));
  file.read(reinterpret_cast<char *>(buffer.data()),fileSize);

  return buffer;
}


ShaderGroup::ShaderGroup(DevicePtr &Device,std::vector<std::filesystem::path> ShaderPaths):mDevice(Device){
  //Load the shader object and descriptor buffer related functions
  pfCreateShader=reinterpret_cast<PFN_vkCreateShadersEXT>(
    vkGetDeviceProcAddr(mDevice->Handle(),"vkCreateShadersEXT"));
  pfDestroyShader=reinterpret_cast<PFN_vkDestroyShaderEXT>(
    vkGetDeviceProcAddr(mDevice->Handle(),"vkDestroyShaderEXT"));

  pfCmdBindShaders=reinterpret_cast<PFN_vkCmdBindShadersEXT>(
    vkGetDeviceProcAddr(mDevice->Handle(),"vkCmdBindShadersEXT"));

  pfGetDescriptorSetLayoutSize=reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(
    vkGetDeviceProcAddr(mDevice->Handle(),"vkGetDescriptorSetLayoutSizeEXT"));

  LoadShaders(ShaderPaths);


  //Put the shaders in shader stage order 
  std::sort(mShaders.begin(),mShaders.end(),[](const ShaderMeta &a,const ShaderMeta &b){
    return a.Stage<b.Stage;
  });


  //Remove deuplicate descriptors from the shader set and remap the new set layouts back to the shaders
  std::map<SetMeta,std::vector<VkPipelineStageFlags>> unique;

  for(auto &shader:mShaders){
    for(auto &[setIndex,set]:shader.SetBindings){
      unique[set].push_back(shader.Stage);

      for(auto &[bindingIndex,name]:set.BindingNames){
        auto pair=std::make_tuple(setIndex,bindingIndex);
        if(mBindingNames.contains(name)){
          if(mBindingNames[name]!=pair)
            throw std::runtime_error("Binding set and location don't match existing names");
        }else
          mBindingNames.insert({name,pair});
      }
    }
  }

  for(auto setIt=unique.begin();setIt!=unique.end();setIt++){
    DescriptorSetLayout::Bindings bindings;

    for(auto &[bindIndex,binding]:setIt->first.Bindings){
      VkDescriptorSetLayoutBinding t=binding.Binding;

      //merge the shader stage flags together
      t.stageFlags=std::accumulate(
        setIt->second.begin(),setIt->second.end(),
        (uint32_t)0,[](uint32_t a,uint32_t b){return a|b;});
      
      bindings.push_back(t);
    }

    auto layouts=DescriptorSetLayout::Create(mDevice,bindings);
    mLayouts.push_back(layouts);

    //assign the descriptor set layout to each shader that uses it
    for(size_t i=0;i<setIt->second.size();i++){
      for(auto &shader:mShaders){
        if(shader.Stage==setIt->second[i])
          shader.Layouts[setIt->first.SetIndex]=layouts;
      }
    }
  }

  CreateShaders(mShaders);
}

ShaderGroup::~ShaderGroup(){
  for(auto &shader:mShaders)
    pfDestroyShader(mDevice->Handle(),shader.ShaderObject,nullptr);
}

void ShaderGroup::LoadShaders(std::vector<std::filesystem::path> &ShaderPaths){
  //Load the shaders and get reflection data for descriptors
  for(auto &path:ShaderPaths){
    auto byteCode=LoadByteCode(path);
    auto reflection=Reflection::Create(byteCode);
    ShaderMeta shader;
    shader.ByteCode=byteCode;
    shader.EntryPoint=reflection->EntryName();
    shader.Stage=reflection->Stage();

    mCompute|=shader.Stage==VK_SHADER_STAGE_COMPUTE_BIT;
    mLink|=bool(shader.Stage&VK_SHADER_STAGE_ALL_GRAPHICS);

    for(auto &b:reflection->DescriptorBindings()){
      if(shader.SetBindings.contains(b.SetIndex)){
        auto &t=shader.SetBindings[b.SetIndex];
        t.Bindings.insert({b.Binding.binding,b});
        t.BindingNames.insert({b.Binding.binding,b.Name});
      } else{
        SetMeta sm;
        sm.SetIndex=b.SetIndex;
        sm.Bindings.insert({b.Binding.binding,b});
        sm.BindingNames.insert({b.Binding.binding,b.Name});

        shader.SetBindings.insert({b.SetIndex,sm});
      }
    }
    mShaders.push_back(shader);
  }

  if(mLink&&mCompute)
    throw std::runtime_error("Can't have a compute shader in graphics pipeline");
}

void ShaderGroup::CreateShaders(std::vector<ShaderMeta> &shaders){
    //Allocate the vulkan shaders
  std::vector<VkShaderCreateInfoEXT> infos;
  std::vector<VkShaderEXT> handles;
  std::vector<std::vector<VkDescriptorSetLayout>> layouts;

  for(size_t i=0;i<shaders.size();i++){
    std::vector<VkDescriptorSetLayout> layoutsTemp;
    for(auto &[setIndex,layout]:shaders[i].Layouts)
      layoutsTemp.push_back(layout->Handle());
    layouts.push_back(layoutsTemp);

    VkShaderStageFlags nextStage=0;
    if(mLink&&i+1!=shaders.size())
      nextStage=shaders[i+1].Stage;


    VkShaderCreateInfoEXT info={
      .sType=VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
      .pNext=nullptr,
      //This is causing an access violation in the amdkv64.dll library 
      .flags=0,//mLink?VK_SHADER_CREATE_LINK_STAGE_BIT_EXT:(uint32_t)0,
      .stage=(VkShaderStageFlagBits)shaders[i].Stage,
      .nextStage=nextStage,
      .codeType=VK_SHADER_CODE_TYPE_SPIRV_EXT,
      .codeSize=(uint32_t)mShaders[i].ByteCode.size()*sizeof(uint32_t),
      .pCode=shaders[i].ByteCode.data(),
      .pName=shaders[i].EntryPoint.c_str(),
      .setLayoutCount=(uint32_t)layouts.back().size(),
      .pSetLayouts=layouts.back().data(),
      .pushConstantRangeCount=0,
      .pPushConstantRanges=nullptr,
      .pSpecializationInfo=nullptr
    };
    infos.push_back(info);
  }

  handles.resize(infos.size());

  auto status=pfCreateShader(mDevice->Handle(),(uint32_t)infos.size(),infos.data(),nullptr,handles.data());
  if(status!=VK_SUCCESS)
    throw std::runtime_error("Unable to create shaders");

  for(int i=0;i<mShaders.size();i++){
    mShaders[i].ShaderObject=handles[i];
    mShaders[i].ByteCode.clear();
  }
}

void ShaderGroup::BindShaders(VkCommandBuffer buffer){
  std::vector<VkShaderEXT> ShaderHandles;
  std::vector<VkShaderStageFlagBits> stageFlags;
  for(auto &shader:mShaders){
    ShaderHandles.push_back(shader.ShaderObject);
    stageFlags.push_back((VkShaderStageFlagBits)shader.Stage);
  }

  //need to also bind unsued stages with nullptr
  pfCmdBindShaders(buffer,(uint32_t)ShaderHandles.size(),stageFlags.data(),ShaderHandles.data());
}

VkDeviceSize ShaderGroup::TotalDescriptorSize(){
  VkDeviceSize size=0;
  VkDeviceSize totalSize=0;

  auto properties=mDevice->GetDescriptorProperties();
  
  VkDeviceSize alignment=properties.descriptorBufferOffsetAlignment;

  for(auto &layout:mLayouts){
    size=0;
    pfGetDescriptorSetLayoutSize(mDevice->Handle(),layout->Handle(),&size);
    totalSize+=MemoryAlignment(size,alignment);
  }
  return totalSize;
}

std::vector<DescriptorSetLayoutPtr> ShaderGroup::GetLayouts(){
  return mLayouts;
}


std::vector<std::string> ShaderGroup::GetVariableNames()const{
  std::vector<std::string> ret;

  for(auto &[name,setBind]:mBindingNames)
    ret.push_back(name);

  return ret;
}

VkDescriptorType ShaderGroup::GetVariableType(const std::string name)const{
  if(mBindingNames.contains(name)){
 
  }
  return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}


struct SetMetaData{
  uint32_t SetIndex;

};

struct ShaderMetaData{
  std::vector<uint32_t> ByteCode;
  ReflectionPtr Reflection;
  VkShaderStageFlagBits Stage;
};


bool operator==(const VkDescriptorSetLayoutBinding &l,const VkDescriptorSetLayoutBinding &r){
  return l.binding==r.binding&&
    l.descriptorType==r.descriptorType&&
    l.descriptorCount==r.descriptorCount;
}


void LoadShaders(std::vector<std::filesystem::path> &ShaderPaths){
  //Load the shaders and get reflection data for descriptors
  std::map<VkShaderStageFlagBits,ShaderMetaData> shaders;
  std::map<uint32_t,VkShaderStageFlags> stagesUsedBySets;


  bool computePipeline=false;
  bool graphicsPipeline=false;
  bool linkFail=false;

  VkShaderStageFlags accumulatedStages=0;

  for(auto &path:ShaderPaths){
    ShaderMetaData shader;
    shader.ByteCode=LoadByteCode(path);
    shader.Reflection=Reflection::Create(shader.ByteCode);
    shader.Stage=shader.Reflection->Stage();

    accumulatedStages|=shader.Stage;

    if(computePipeline){
      if(shader.Stage==VK_SHADER_STAGE_COMPUTE_BIT)
        throw std::runtime_error("Duplicate compute shader found");
      throw std::runtime_error("Compute pipeline can only have a single compute shader");
    } else
      computePipeline|=shader.Stage==VK_SHADER_STAGE_COMPUTE_BIT;

    if(graphicsPipeline){
      if(shader.Stage==VK_SHADER_STAGE_COMPUTE_BIT)
        throw std::runtime_error("Graphic pipeline can not have a compute shader");
    } else
      graphicsPipeline|=(bool)(shader.Stage&VK_SHADER_STAGE_ALL_GRAPHICS);

    for(auto &binding:shader.Reflection->DescriptorBindings()){
      stagesUsedBySets[binding.SetIndex]|=shader.Stage;
    }

    if(shaders.contains(shader.Stage))
      throw std::runtime_error("Uplicate shader for stage found");

    shaders.insert({shader.Stage,shader});
  }

  for(auto &[unsued,stage]:stagesUsedBySets)
    linkFail|=stage!=accumulatedStages;
    
 
}