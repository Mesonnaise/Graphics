#include "FastGraphicPipeline.h"
#include"Util.h"
#include"Internal/VulkanFunctions.h"

namespace Engine{
  FastGraphicPipeline::FastGraphicPipeline(DevicePtr device,AllocatorPtr allocator,std::vector<std::filesystem::path> shaders,bool flipY):
    FastPipeline(device,allocator,shaders),mFlipY(flipY){}

  void FastGraphicPipeline::SetFrameBuffer(BaseImagePtr image,ImageViewPtr view){
    mOutputImage=image;
    mOutputImageView=view;
  }

  void FastGraphicPipeline::PopulateCommandBuffer(
    VkCommandBuffer CMDBuffer,
    uint32_t vertexCount,uint32_t instanceCount,
    uint32_t firstVertex,uint32_t firstInstance){

    auto extent=mOutputImage->Extent();

    BuildDescriptorBuffer();

    float height=(float)extent.height;

    VkViewport viewPort={
      .x=0.0f,
      .y=0.0f,
      .width=(float)extent.width,
      .height=height,
      .minDepth=0.0f,
      .maxDepth=1.0f
    };

    if(mFlipY){
      viewPort.y=height;
      viewPort.height=-height;
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

    VkRenderingAttachmentInfo attachmentInfo={
      .sType=VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext=nullptr,
      .imageView=mOutputImageView->Handle(),
      .imageLayout=VK_IMAGE_LAYOUT_GENERAL,
      .resolveMode=VK_RESOLVE_MODE_NONE,
      .resolveImageView=VK_NULL_HANDLE,
      .resolveImageLayout=VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp=VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue={.color={
        mClearColor[0],mClearColor[1],
        mClearColor[2],mClearColor[3]
      }}
    };

    if(mClearColor[3]==0.0f)
      attachmentInfo.loadOp=VK_ATTACHMENT_LOAD_OP_LOAD;

    /*
    VkRenderingAttachmentInfo attachmentDepthInfo={
      .sType=VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext=nullptr,
      .imageView=mOutputImageViewDepth->Handle(),
      .imageLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .resolveMode=VK_RESOLVE_MODE_NONE,
      .resolveImageView=VK_NULL_HANDLE,
      .resolveImageLayout=VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp=VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .clearValue={.depthStencil{.depth=1.0f}}
    };
    */
    VkRect2D scissor={
      .offset={0,0},
      .extent={extent.width,extent.height}
    };
    VkRenderingInfo renderingInfo={
      .sType=VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext=nullptr,
      .flags=0,
      .renderArea={
        .offset={0,0},
        .extent={extent.width,extent.height}
      },
      .layerCount=1,
      .viewMask=0,
      .colorAttachmentCount=1,
      .pColorAttachments=&attachmentInfo,
      .pDepthAttachment=nullptr//&attachmentDepthInfo
    };

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