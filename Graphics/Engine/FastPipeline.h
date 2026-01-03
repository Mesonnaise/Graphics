#pragma once
#include<memory>
#include<string>
#include<vulkan/vulkan.h>
#include"ShaderObject.h"
#include"PipelineLayout.h"
#include"Buffer.h"
#include"Image.h"
class FastPipeline{
  ShaderObjectPtr mShaderObject=nullptr;
  PipelineLayoutPtr mLayout=nullptr;

public:
  virtual BufferPtr GetBuffer(std::string name);
  virtual ImagePtr GetImage(std::string name);
  virtual void BuildCommandBuffer(VkCommandBuffer CMDBuffer)=0;
};

