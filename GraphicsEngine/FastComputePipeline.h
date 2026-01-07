#pragma once
#include<filesystem>
#include<memory>
#include<map>
#include"Device.h"
#include"Allocator.h"
#include"ShaderObject.h"
#include"PipelineLayout.h"
#include"FastPipeline.h"

#include"Buffer.h"
#include"Image.h"
#include"ImageView.h"

namespace Engine{
  class FastComputePipeline:public FastPipeline{
  protected:
    FastComputePipeline(DevicePtr &device,AllocatorPtr &allocator,std::filesystem::path &ShaderPath);

  public:
    static inline std::shared_ptr<FastComputePipeline> Create(DevicePtr &device,AllocatorPtr &allocator,std::filesystem::path ShaderPath){
      auto p=new FastComputePipeline(device,allocator,ShaderPath);
      return std::shared_ptr<FastComputePipeline>(p);
    }

    void PopulateCommandBuffer(VkCommandBuffer CMDBuffer,uint32_t x,uint32_t y,uint32_t z);
  };

  using FastComputePipelinePtr=std::shared_ptr<FastComputePipeline>;
}