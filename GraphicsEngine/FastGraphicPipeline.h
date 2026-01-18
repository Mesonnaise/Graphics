#pragma once
#include<array>
#include<memory>
#include<filesystem>
#include<functional>

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
  private:

    BufferPtr mVertexBuffer=nullptr;

    std::vector<VkRenderingAttachmentInfo> mAttachments;
    std::vector<ImageViewPtr> mAttachmentsView;

    ImageViewPtr mDepthAttachmentView=nullptr;
    VkRenderingAttachmentInfo mDepthAttachment={};
    ImageViewPtr mStencilAttachmentView=nullptr;
    VkRenderingAttachmentInfo mStencilAttachment={};



    std::array<float,4> mClearColor={1.0f,1.0f,1.0f,1.0f};
    bool mFlipY=false;
    VkExtent2D mViewport;

  protected:
    FastGraphicPipeline(DevicePtr device,AllocatorPtr allocator,std::vector<std::filesystem::path> shaders,bool flipY);

  public:
    static inline std::shared_ptr<FastGraphicPipeline> Create(
      DevicePtr device,AllocatorPtr allocator,
      std::vector<std::filesystem::path> shaders,bool flipY=false){

      auto p=new FastGraphicPipeline(device,allocator,shaders,flipY);
      return std::shared_ptr<FastGraphicPipeline>(p);
    }

    void AddAttachment(ImageViewPtr view);
    void AddAttachment(ImageViewPtr view, VkRenderingAttachmentInfo attachment);
    void AddDepthAttachment(ImageViewPtr view);
    void AddStencilAttachment(ImageViewPtr view);
    void ClearAttachments();
    inline void SetViewport(VkExtent2D viewport){
      mViewport=viewport;
    }
    inline void SetViewport(VkExtent3D viewport){
      mViewport.height=viewport.height;
      mViewport.width=viewport.width;
    }

    inline void AssignVertexBuffer(BufferPtr &buffer){
      mVertexBuffer=buffer;
    }

    void PopulateCommandBuffer(
      VkCommandBuffer CMDBuffer,
      uint32_t vertexCount,uint32_t instanceCount,
      uint32_t firstVertex,uint32_t firstInstance);
  };

  using FastGraphicPipelinePtr=std::shared_ptr<FastGraphicPipeline>;
}

