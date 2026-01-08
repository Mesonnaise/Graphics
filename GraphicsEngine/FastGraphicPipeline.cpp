#include "FastGraphicPipeline.h"
#include"Util.h"
#include"Internal/VulkanFunctions.h"

namespace Engine{
  FastGraphicPipeline::FastGraphicPipeline(DevicePtr device,AllocatorPtr allocator,std::vector<std::filesystem::path> shaders,bool flipY):
    FastPipeline(device,allocator,shaders),mFlipY(flipY){}

  void FastGraphicPipeline::AddAttachment(ImageViewPtr view){
    AddAttachment(view,view->BasicAttachment(VK_IMAGE_LAYOUT_GENERAL));
  }

  void FastGraphicPipeline::AddAttachment(ImageViewPtr view,VkRenderingAttachmentInfo attachment){
    mAttachmentsView.push_back(view);
    attachment.imageView=view->Handle();
    mAttachments.push_back(attachment);
  }

  void FastGraphicPipeline::AddDepthAttachment(ImageViewPtr view){
    mDepthAttachmentView=view;
    mDepthAttachment=view->BasicAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    mDepthAttachment.storeOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    mDepthAttachment.clearValue.depthStencil.depth=1.0f;
  }

  void FastGraphicPipeline::AddStencilAttachment(ImageViewPtr view){
    mDepthAttachmentView=view;
    mDepthAttachment=view->BasicAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    mDepthAttachment.storeOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    mDepthAttachment.clearValue.depthStencil.depth=1.0f;
  }

  void FastGraphicPipeline::ClearAttachments(){
    mAttachmentsView.clear();
    mAttachments.clear();
  }

  void FastGraphicPipeline::PopulateCommandBuffer(
    VkCommandBuffer CMDBuffer,
    uint32_t vertexCount,uint32_t instanceCount,
    uint32_t firstVertex,uint32_t firstInstance){

    BuildDescriptorBuffer();

    VkViewport viewPort={
      .x=0.0f,
      .y=0.0f,
      .width=(float)mViewport.width,
      .height=(float)mViewport.height,
      .minDepth=0.0f,
      .maxDepth=1.0f
    };

    if(mFlipY){
      viewPort.y=viewPort.height;
      viewPort.height*=-1.0f;
    }

    VkVertexInputBindingDescription2EXT vertexInputBinding={
      .sType=VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
      .pNext=nullptr,
      .binding=0,
      .stride=sizeof(glm::vec3),
      .inputRate=VK_VERTEX_INPUT_RATE_VERTEX,
      .divisor=1
    };

    VkVertexInputAttributeDescription2EXT vertexInputAttribute{
      .sType=VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
      .pNext=nullptr,
      .location=0,
      .binding=0,
      .format=VK_FORMAT_R32G32B32_SFLOAT,
      .offset=0
    };

    VkRect2D scissor={
      .offset={0,0},
      .extent=mViewport
    };
    VkRenderingInfo renderingInfo={
      .sType=VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext=nullptr,
      .flags=0,
      .renderArea={
        .offset={0,0},
        .extent=mViewport
      },
      .layerCount=1,
      .viewMask=0,
      .colorAttachmentCount=(uint32_t)mAttachments.size(),
      .pColorAttachments=mAttachments.data(),
      .pDepthAttachment=nullptr//&attachmentDepthInfo
    };

    if(mDepthAttachmentView!=nullptr)
      renderingInfo.pDepthAttachment=&mDepthAttachment;
    
    if(mStencilAttachmentView!=nullptr)
      renderingInfo.pStencilAttachment=&mStencilAttachment;


    std::vector<VkDeviceSize> layoutOffsets;
    std::vector<uint32_t> layoutIndecies;
    VkDeviceSize currentOffset=0;
    for(auto layout:mShaderObject->GetLayouts()){
      layoutOffsets.push_back(currentOffset);
      currentOffset+=layout->GetLayoutSize();
      layoutIndecies.push_back(0);
    }

    vkCmdBeginRendering(CMDBuffer,&renderingInfo);
    BasicGraphicsPipeline(CMDBuffer);


    vkCmdSetViewportWithCount(CMDBuffer,1,&viewPort);
    vkCmdSetScissorWithCount(CMDBuffer,1,&scissor);
    pfnCmdSetVertexInputEXT(CMDBuffer,1,&vertexInputBinding,1,&vertexInputAttribute);

    mLayout->BindShaders(CMDBuffer);
    VkDeviceSize offsets[]={0};
    auto vertexBuffer=mVertexBuffer->Handle();
    vkCmdBindVertexBuffers(CMDBuffer,0,1,&vertexBuffer,offsets);
    mDescriptorBuffer->CommandBufferBind(CMDBuffer);
    mLayout->SetDescriptorBufferOffsets(CMDBuffer,layoutIndecies,layoutOffsets);

    vkCmdDraw(CMDBuffer,vertexCount,instanceCount,firstVertex,firstInstance);
    vkCmdEndRendering(CMDBuffer);
  }
}