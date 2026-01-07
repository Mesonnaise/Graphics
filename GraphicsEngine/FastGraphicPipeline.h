#pragma once
#include<array>
#include<memory>
#include<filesystem>
#include"Device.h"
#include"Allocator.h"
#include"ShaderObject.h"
#include"PipelineLayout.h"
#include"Buffer.h"
#include"Image.h"
#include"ImageView.h"
#include"FastPipeline.h"

namespace Engine{
  class FastGraphicPipeline:public FastPipeline{
    BufferPtr mVertexBuffer=nullptr;
    BaseImagePtr mOutputImage=nullptr;
    ImageViewPtr mOutputImageView=nullptr;
    ImageViewPtr mOutputImageViewDepth=nullptr;
    std::array<float,4> mClearColor={1.0f,1.0f,1.0f,1.0f};
    bool mFlipY=false;

  protected:
    FastGraphicPipeline(DevicePtr device,AllocatorPtr allocator,std::vector<std::filesystem::path> shaders,bool flipY);

  public:
    static inline std::shared_ptr<FastGraphicPipeline> Create(
      DevicePtr device,AllocatorPtr allocator,
      std::vector<std::filesystem::path> shaders,bool flipY=false){

      auto p=new FastGraphicPipeline(device,allocator,shaders,flipY);
      return std::shared_ptr<FastGraphicPipeline>(p);
    }

    inline void AssignClearColor(std::array<float,4> color){
      mClearColor=color;
    }

    inline void AssignVertexBuffer(BufferPtr &buffer){
      mVertexBuffer=buffer;
    }

    void SetFrameBuffer(BaseImagePtr image,ImageViewPtr view);
    void PopulateCommandBuffer(
      VkCommandBuffer CMDBuffer,
      uint32_t vertexCount,uint32_t instanceCount,
      uint32_t firstVertex,uint32_t firstInstance);

  };

  using FastGraphicPipelinePtr=std::shared_ptr<FastGraphicPipeline>;
}

