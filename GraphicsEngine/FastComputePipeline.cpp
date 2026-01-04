#include "FastComputePipeline.h"

namespace Engine{
  FastComputePipeline::FastComputePipeline(DevicePtr &device,AllocatorPtr &allocator,std::filesystem::path &ShaderPath):
    mDevice(device),mAllocator(allocator){

    std::vector<VkPushConstantRange> pushRanges;
    mShaderOjbect=ShaderObject::Create(mDevice,{ShaderPath});

    if(mShaderOjbect->HashPushConstant()){
      mPushConstantSize=mShaderOjbect->PushConstantSize();
      VkPipelineStageFlags stages=0;
      for(auto stage:mShaderOjbect->Stages())
        stages|=stage;

      VkPushConstantRange range={
        .stageFlags=stages,
        .offset=0,
        .size=mPushConstantSize
      };
      pushRanges.push_back(range);
    }
    mLayout=PipelineLayout::Create(mDevice,{mShaderOjbect},pushRanges);
    mDescriptorBuffer=mAllocator->CreateDescriptorBuffer(
      mShaderOjbect->GetDescriptorBufferTotalSize(),true);
  }

  void FastComputePipeline::BuildDescriptorBuffer(){
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
          throw std::runtime_error("FastComputePipeline::BuildDescriptorBuffer: Buffer "+name+" not assigned!");

        offset+=mBuffers[name]->GetDescriptor(mapping+offset,descriptorType);

      } else if(descriptorType==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
        descriptorType==VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_IMAGE){


        if(mImages.contains(name)==false)
          throw std::runtime_error("FastComputePipeline::BuildDescriptorBuffer: Image "+name+" not assigned!");

        offset+=mImageViews[name]->GetDescriptor(mapping+offset,descriptorType,mImages[name]->GetLayout());
      }
    }

    mRebuildDescriptorBuffer=false;
  }

  void FastComputePipeline::QuickCreateBuffers(){
    for(auto name:mShaderOjbect->GetVariableNames()){
      VkDescriptorSetLayoutBinding binding=mShaderOjbect->VariableBinding(name);
      VkDescriptorType descriptorType=binding.descriptorType;

      if(descriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_BUFFER){

        auto size=mShaderOjbect->VariableStructureSize(name);
        VkBufferUsageFlags usage=VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        switch(descriptorType){
          case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            usage|=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
          case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            usage|=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
          default:
            throw std::runtime_error("FastComputePipeline::QuickCreateBuffers: Unsupported buffer usage");
        }

        auto buffer=mAllocator->CreateBuffer(size,usage,true);
        mBuffers[name]=buffer;
      }
    }
  }

  void FastComputePipeline::PopulateCommandBuffer(VkCommandBuffer CMDBuffer,uint32_t x,uint32_t y,uint32_t z){
    BuildDescriptorBuffer();

    std::vector<VkDeviceSize> layoutOffsets;
    std::vector<uint32_t> layoutIndecies;
    VkDeviceSize currentOffset=0;
    for(auto layout:mShaderOjbect->GetLayouts()){
      layoutOffsets.push_back(currentOffset);
      currentOffset+=layout->GetLayoutSize();
      layoutIndecies.push_back(0);
    }

    mLayout->BindShaders(CMDBuffer);
    mDescriptorBuffer->CommandBufferBind(CMDBuffer);
    mLayout->SetDescriptorBufferOffsets(CMDBuffer,layoutIndecies,layoutOffsets);

    if(mPushConstantSize){
      if(mPushConstantSize!=mPushConstantData.size())
        throw std::runtime_error(std::format("Push constant size {} bytes, instead of {} bytes",mPushConstantData.size(),mPushConstantSize));

      VkShaderStageFlags stageFlags=0;
      for(auto stage:mShaderOjbect->Stages())
        stageFlags|=stage;

      VkPushConstantsInfo pushInfo={
        .sType=VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
        .pNext=nullptr,
        .layout=mLayout->Handle(),
        .stageFlags=stageFlags,
        .offset=0,
        .size=(uint32_t)mPushConstantData.size(),
        .pValues=mPushConstantData.data()
      };

      vkCmdPushConstants2(CMDBuffer,&pushInfo);
    }
    vkCmdDispatch(CMDBuffer,x,y,z);
  }
}