#include<format>
#include<string>
#include<filesystem>
#include<fstream>
#include<iostream>
#include<memory>
#include<map>
#include<vulkan/vulkan.h>
#include<spirv_cross/spirv_glsl.hpp>

using CrossRes=spirv_cross::SmallVector<spirv_cross::Resource>;
using CompilerPtr=std::unique_ptr<spirv_cross::Compiler>;

struct ShaderStage{
  std::filesystem::path SPIRVPath;
  std::string EntryName;
  VkShaderStageFlags Stage;

  std::map<uint32_t,VkDescriptorSetLayoutBinding> Bindings;
  std::vector<VkPushConstantRange> PushconstantRanges;
};

static auto CreateBinding(CompilerPtr &Compiler,CrossRes Resources,VkDescriptorType DescriptorType,VkShaderStageFlags Stage){
  std::unordered_multimap<uint32_t,VkDescriptorSetLayoutBinding> ret;

  for(auto &res:Resources){
    VkDescriptorSetLayoutBinding binding;
    auto type=Compiler->get_type(res.type_id);
    auto baseType=Compiler->get_type(res.base_type_id);

    uint32_t set=Compiler->get_decoration(res.id,spv::DecorationDescriptorSet);

    binding.descriptorType=DescriptorType;
    binding.binding=Compiler->get_decoration(res.id,spv::DecorationBinding);
    binding.stageFlags=Stage;

    if(DescriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      binding.descriptorCount=Compiler->get_declared_struct_size(baseType);
    else
      binding.descriptorCount=type.array.size();

    ret.insert({set,binding});
  }


  return ret;
}


static std::vector<uint32_t> LoadByteCode(std::filesystem::path path){
  std::ifstream file(path,std::ios::binary);
  std::vector<uint32_t> buffer;

  file.seekg(0,file.end);
  size_t fileSize=file.tellg();
  file.seekg(0,file.beg);

  buffer.resize(fileSize/sizeof(uint32_t));
  file.read(reinterpret_cast<char *>(buffer.data()),fileSize);

  return buffer;
}




int main0(){
  std::vector<std::filesystem::path> ShaderPaths;
  std::vector<ShaderStage> Stages;


  for(auto shaderPath:ShaderPaths){
    ShaderStage stage;
    if(!std::filesystem::exists(shaderPath)){
      std::cerr<<std::format("Unable to file find file {}\n",shaderPath.string());
      return -1;
    }

    auto compiler=std::make_unique<spirv_cross::Compiler>(LoadByteCode(shaderPath));
    for(auto &entry:compiler->get_entry_points_and_stages()){
      stage.EntryName=entry.name;
      stage.Stage=entry.execution_model;
    }

    auto resources=compiler->get_shader_resources();
    
    auto bindings=CreateBinding(compiler,resources.uniform_buffers,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,stage.Stage);
    stage.Bindings.insert_range(bindings);

    uint32_t pushOffset=0;

    for(auto &push:resources.push_constant_buffers){
      VkPushConstantRange range;
      auto type=compiler->get_type(push.base_type_id);
      auto size=compiler->get_declared_struct_size(type);

      range.size=size;
      range.offset=pushOffset;
      range.stageFlags=stage.Stage;

      pushOffset+=size;
      stage.PushconstantRanges.push_back(range);
    }

    Stages.push_back(stage);
  }





  return 0;

}