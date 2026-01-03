#include "FastGraphicPipeline.h"
#include"Util.h"

namespace Engine{
  FastGraphicPipeline::FastGraphicPipeline(DevicePtr device,AllocatorPtr allocator,std::vector<std::filesystem::path> shaders):
    mDevice(device),mAllocator(allocator){

    mShaderOjbect=ShaderObject::Create(mDevice,shaders);
    mLayout=PipelineLayout::Create(mDevice,{mShaderOjbect});
    mDescriptorBuffer=mAllocator->CreateDescriptorBuffer(
      mShaderOjbect->GetDescriptorBufferTotalSize(),true);
  }

  void FastGraphicPipeline::BuildDescriptorBuffer(){
    if(!mRebuildDescriptorBuffer)
      return;

    size_t offset=0;
    uint8_t *mapping=reinterpret_cast<uint8_t *>(mDescriptorBuffer->Mapped());

    for(auto name:mShaderOjbect->GetVariableNames()){
      VkDescriptorSetLayoutBinding binding=mShaderOjbect->VariableBinding(name);
      VkDescriptorType descriptorType=binding.descriptorType;

      if(descriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_BUFFER){

        if(mBuffers.contains(name)==false)
          throw std::runtime_error("FastGraphicPipeline::BuildDescriptorBuffer: Buffer "+name+" not assigned!");

        offset+=mBuffers[name]->GetDescriptor(mapping+offset,descriptorType);

      } else if(descriptorType==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
        descriptorType==VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_IMAGE){

        if(mImages.contains(name)==false)
          throw std::runtime_error("FastGraphicPipeline::BuildDescriptorBuffer: Image "+name+" not assigned!");

        offset+=mImageViews[name]->GetDescriptor(mapping+offset,descriptorType,mImages[name]->GetLayout());
      }
    }
    mRebuildDescriptorBuffer=false;
  }

  void FastGraphicPipeline::QuickCreateBuffers(){
    for(auto name:mShaderOjbect->GetVariableNames()){
      VkDescriptorSetLayoutBinding binding=mShaderOjbect->VariableBinding(name);
      VkDescriptorType descriptorType=binding.descriptorType;

      auto size=mShaderOjbect->VariableDescriptorSize(name);

      if(descriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_BUFFER){
        auto buffer=mAllocator->CreateBuffer(size,descriptorType);
        mBuffers[name]=buffer;
      }
    }
  }

  void FastGraphicPipeline::PopulateCommandBuffer(
    VkCommandBuffer CMDBuffer,
    uint32_t vertexCount,uint32_t instanceCount,
    uint32_t firstVertex,uint32_t firstInstance){

    uint32_t ViewWidth=640;
    uint32_t ViewHeight=480;

    BuildDescriptorBuffer();

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
      .clearValue={.color={1.0f,0.0f,0.0f,1.0f}}
    };
    VkRect2D scissor={
      .offset={0,0},
      .extent={ViewWidth,ViewHeight}
    };
    VkRenderingInfo renderingInfo={
      .sType=VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext=nullptr,
      .flags=0,
      .renderArea={
        .offset={0,0},
        .extent={ViewWidth,ViewHeight}
      },
      .layerCount=1,
      .viewMask=0,
      .colorAttachmentCount=1,
      .pColorAttachments=&attachmentInfo,
    };

    vkCmdBeginRendering(CMDBuffer,&renderingInfo);
    BasicGraphicsPipeline(CMDBuffer);

    mLayout->BindShaders(CMDBuffer);
    VkDeviceSize offsets[]={0};
    auto vertexBuffer=mVertexBuffer->Handle();
    vkCmdBindVertexBuffers(CMDBuffer,0,1,&vertexBuffer,offsets);
    mDescriptorBuffer->CommandBufferBind(CMDBuffer);

  }
}