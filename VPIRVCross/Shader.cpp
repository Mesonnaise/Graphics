#include<filesystem>
#include<fstream>

#include<spirv_cross/spirv_cpp.hpp>

#include "Shader.h"



static std::vector<uint32_t> LoadByteCode(std::filesystem::path &path){
  std::ifstream file(path,std::ios::binary);
  std::vector<uint32_t> buffer;

  file.seekg(0,file.end);
  size_t fileSize=file.tellg();
  file.seekg(0,file.beg);

  buffer.resize(fileSize/sizeof(uint32_t));
  file.read(reinterpret_cast<char *>(buffer.data()),fileSize);

  return buffer;
}


struct Validator{
  spirv_cross::Compiler mCompiler;

  Validator(std::vector<uint32_t> ByteCode):mCompiler(ByteCode){

  }

  

};


Shader::Shader(DevicePtr &Device,PathGroup Group):mDevice(Device){
  std::vector<VkShaderCreateInfoEXT> info;




 // auto status=vkCreateShadersEXT(mDevice->Handle(),(uint32_t)info.size(),info.data(),nullptr,&mHandle);
 // if(status!=VK_SUCCESS)
 //   throw std::runtime_error("Unable to create shader object");
}

Shader::~Shader(){
 // vkDestroyShaderEXT(mDevice->Handle(),mHandle,nullptr);
}