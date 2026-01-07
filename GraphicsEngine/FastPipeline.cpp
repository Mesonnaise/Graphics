#include "FastPipeline.h"

namespace Engine{
  FastPipeline::FastPipeline(DevicePtr &device,AllocatorPtr &allocator,std::vector<std::filesystem::path> shaderPaths):
    mDevice(device),mAllocator(allocator){

    std::vector<VkPushConstantRange> pushRanges;
    mShaderObject=ShaderObject::Create(mDevice,shaderPaths);

    if(mShaderObject->HashPushConstant()){
      mPushConstantSize=mShaderObject->PushConstantSize();
      VkPipelineStageFlags stages=0;
      for(auto stage:mShaderObject->Stages())
        stages|=stage;

      VkPushConstantRange range={
        .stageFlags=stages,
        .offset=0,
        .size=mPushConstantSize
      };
      pushRanges.push_back(range);
    }

    mLayout=PipelineLayout::Create(mDevice,{mShaderObject},pushRanges);
    mDescriptorBuffer=mAllocator->CreateDescriptorBuffer(
      mShaderObject->GetDescriptorBufferTotalSize(),true);
  }


  void FastPipeline::BuildDescriptorBuffer(){
    if(!mRebuildDescriptorBuffer)
      return;

    size_t offset=0;
    uint8_t *mapping=reinterpret_cast<uint8_t *>(mDescriptorBuffer->Mapped());

    for(auto name:mShaderObject->GetVariableNames()){
      VkDescriptorSetLayoutBinding binding=mShaderObject->VariableBinding(name);
      VkDescriptorType descriptorType=binding.descriptorType;

      if(descriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_BUFFER){

        if(mBuffers.contains(name)==false)
          throw std::runtime_error("FastPipeline::BuildDescriptorBuffer: Buffer "+name+" not assigned!");

        offset+=mBuffers[name]->GetDescriptor(mapping+offset,descriptorType);

      } else if(descriptorType==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
        descriptorType==VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_IMAGE){

        if(mImages.contains(name)==false)
          throw std::runtime_error("FastPipeline::BuildDescriptorBuffer: Image "+name+" not assigned!");

        offset+=mImageViews[name]->GetDescriptor(mapping+offset,descriptorType,mImages[name]->GetLayout());
      }
    }
    mRebuildDescriptorBuffer=false;
  }


  void FastPipeline::QuickCreateBuffers(){
    for(auto name:mShaderObject->GetVariableNames()){
      VkDescriptorSetLayoutBinding binding=mShaderObject->VariableBinding(name);
      VkDescriptorType descriptorType=binding.descriptorType;

      if(descriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER||
        descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_BUFFER){

        auto size=mShaderObject->VariableStructureSize(name);
        VkBufferUsageFlags usage=VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        switch(descriptorType){
          case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            usage|=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
          case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            usage|=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
          default:
            throw std::runtime_error("FastPipeline::QuickCreateBuffers: Unsupported buffer usage");
        }

        auto buffer=mAllocator->CreateBuffer(size,usage,true);
        mBuffers[name]=buffer;
      }
    }
  }

  std::vector<std::string> FastPipeline::GetVariableNames(){
    return mShaderObject->GetVariableNames();
  }

  BufferPtr FastPipeline::GetBuffer(std::string name){
    if(mBuffers.find(name)==mBuffers.end())
      return nullptr;
    return mBuffers[name];
  }

  BaseImagePtr FastPipeline::GetImage(std::string name){
    if(mImages.find(name)==mImages.end())
      return nullptr;
    return mImages[name];
  }

  void FastPipeline::AssignBuffer(std::string name,BufferPtr buffer){
    mBuffers[name]=buffer;
    mRebuildDescriptorBuffer=true;
  }

  void FastPipeline::AssignImage(std::string name,BaseImagePtr image,ImageViewPtr view){
    mImages[name]=image;
    mImageViews[name]=view;
    mRebuildDescriptorBuffer=true;
  }
}
