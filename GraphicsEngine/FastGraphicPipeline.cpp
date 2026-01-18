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
    mDepthAttachment.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
    mDepthAttachment.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
    mDepthAttachment.clearValue.depthStencil.depth=1.0f;
  }

  void FastGraphicPipeline::AddStencilAttachment(ImageViewPtr view){
    mDepthAttachmentView=view;
    mDepthAttachment=view->BasicAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    mDepthAttachment.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
    mDepthAttachment.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
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

    UpdateDescriptors();

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
      .pDepthAttachment=nullptr,
      .pStencilAttachment=nullptr
    };

    if(mDepthAttachmentView!=nullptr)
      renderingInfo.pDepthAttachment=&mDepthAttachment;
    
    if(mStencilAttachmentView!=nullptr)
      renderingInfo.pStencilAttachment=&mStencilAttachment;

    vkCmdBeginRendering(CMDBuffer,&renderingInfo);
    
    VkSampleMask SampleMask=0x1;
    VkBool32 vkFalse=VK_FALSE;
    VkColorComponentFlags MaskColorComponent=
      VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|
      VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;

    vkCmdSetDepthTestEnable(CMDBuffer,VK_FALSE);
    vkCmdSetDepthBiasEnable(CMDBuffer,VK_FALSE);
    pfnCmdSetDepthClampEnableEXT(CMDBuffer,VK_FALSE);
    //vkCmdSetDepthWriteEnable(CMDBuffer,VK_TRUE);
    //vkCmdSetDepthCompareOp(CMDBuffer,VK_COMPARE_OP_LESS);
    
    vkCmdSetDepthBoundsTestEnable(CMDBuffer,VK_FALSE);
    

    vkCmdSetStencilTestEnable(CMDBuffer,VK_FALSE);
    //vkCmdSetStencilCompareMask(CMDBuffer,VK_STENCIL_FACE_FRONT_BIT,1);
    //vkCmdSetStencilWriteMask(CMDBuffer,VK_STENCIL_FACE_FRONT_BIT,1);
    //vkCmdSetStencilReference(CMDBuffer,VK_STENCIL_FACE_FRONT_BIT,1);
    //vkCmdSetStencilOp(
    //  CMDBuffer,VK_STENCIL_FACE_FRONT_BIT,
    //  VK_STENCIL_OP_REPLACE,VK_STENCIL_OP_REPLACE,
    //  VK_STENCIL_OP_REPLACE,VK_COMPARE_OP_LESS);

    vkCmdSetCullMode(CMDBuffer,VK_CULL_MODE_BACK_BIT);
    vkCmdSetRasterizerDiscardEnable(CMDBuffer,VK_FALSE);
    vkCmdSetFrontFace(CMDBuffer,VK_FRONT_FACE_COUNTER_CLOCKWISE);

    
    pfnCmdSetPolygonModeEXT(CMDBuffer,VK_POLYGON_MODE_FILL);
    pfnCmdSetRasterizationSamplesEXT(CMDBuffer,VK_SAMPLE_COUNT_1_BIT);
    pfnCmdSetSampleMaskEXT(CMDBuffer,VK_SAMPLE_COUNT_1_BIT,&SampleMask);
    
    pfnCmdSetAlphaToCoverageEnableEXT(CMDBuffer,VK_FALSE);
    pfnCmdSetLogicOpEnableEXT(CMDBuffer,VK_FALSE);
    pfnCmdSetColorBlendEnableEXT(CMDBuffer,0,1,&vkFalse);
    pfnCmdSetColorWriteMaskEXT(CMDBuffer,0,1,&MaskColorComponent);
    vkCmdSetPrimitiveTopology(CMDBuffer,VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pfnCmdSetProvokingVertexModeEXT(CMDBuffer,VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT);
    vkCmdSetPrimitiveRestartEnable(CMDBuffer,VK_FALSE);


    vkCmdSetViewportWithCount(CMDBuffer,1,&viewPort);
    vkCmdSetScissorWithCount(CMDBuffer,1,&scissor);
    pfnCmdSetVertexInputEXT(CMDBuffer,1,&vertexInputBinding,1,&vertexInputAttribute);

    auto vb=mVertexBuffer->Handle();
    VkDeviceSize vbOffset=0;
    VkDeviceSize vbSize=mVertexBuffer->AllocatedSize();
    VkDeviceSize vbStride=sizeof(glm::vec3);
    vkCmdBindVertexBuffers2(CMDBuffer,0,1,&vb, &vbOffset, &vbSize,&vbStride);

    mLayout->BindShaders(CMDBuffer);
    WriteDescriptors(CMDBuffer);

    vkCmdDraw(CMDBuffer,vertexCount,instanceCount,firstVertex,firstInstance);
    vkCmdEndRendering(CMDBuffer);
  }
}