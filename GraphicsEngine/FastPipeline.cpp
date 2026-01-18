#include "FastPipeline.h"

namespace Engine{
  FastPipeline::FastPipeline(DevicePtr &device,AllocatorPtr &allocator,std::vector<std::filesystem::path> shaderPaths):
    mDevice(device),mAllocator(allocator){

    std::vector<VkPushConstantRange> pushRanges;
    mShaderObject=ShaderObject::Create(mDevice,shaderPaths,mUseBuffer);

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
    if(mUseBuffer){
      mDescriptorBuffer=mAllocator->CreateDescriptorBuffer(
        mShaderObject->GetDescriptorBufferTotalSize(),true);

    } else{
      std::vector<VkDescriptorSetLayout> setLayouts;
      for(auto l:mShaderObject->GetLayouts())
        setLayouts.push_back(l->Handle());

      mDescriptorPool=DescriptorPool::Create(mDevice,32);
      mDescriptorSets=mDescriptorPool->Allocate(setLayouts);
    }
  }

  void FastPipeline::UpdateDescriptors(){
    if(!mRebuildDescriptors)
      return;

    if(mUseBuffer){
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
    } else{
      std::vector<VkDescriptorImageInfo> descImages;
      std::vector<VkDescriptorBufferInfo> descBuffers;
      std::vector<VkWriteDescriptorSet> writeDescriptorSets;

      for(auto name:mShaderObject->GetVariableNames()){
        auto set=mDescriptorSets[mShaderObject->VariableSetIndex(name)];
        auto binding=mShaderObject->VariableBinding(name);

        VkWriteDescriptorSet setInfo={
          .sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .pNext=nullptr,
          .dstSet=set,
          .dstBinding=binding.binding,
          .dstArrayElement=0,
          .descriptorCount=1,
          .descriptorType=binding.descriptorType,
          .pImageInfo=nullptr,
          .pBufferInfo=nullptr,
          .pTexelBufferView=nullptr
        };

        if(binding.descriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER||
          binding.descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_BUFFER){
          auto &buffer=mBuffers.at(name);

          VkDescriptorBufferInfo info={
            .buffer=buffer->Handle(),
            .offset=0,
            .range=VK_WHOLE_SIZE,
          };

          descBuffers.push_back(info);
          setInfo.pBufferInfo=&descBuffers.back();
        } else if(binding.descriptorType==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
          binding.descriptorType==VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE||
          binding.descriptorType==VK_DESCRIPTOR_TYPE_STORAGE_IMAGE){

          VkDescriptorImageInfo info={
            .sampler=nullptr,
            .imageView=mImageViews.at(name)->Handle(),
            .imageLayout=mImages.at(name)->GetLayout()
          };
          descImages.push_back(info);
          setInfo.pImageInfo=&descImages.back();
        }

        writeDescriptorSets.push_back(setInfo);
      }



      vkUpdateDescriptorSets(mDevice->Handle(),(uint32_t)writeDescriptorSets.size(),writeDescriptorSets.data(),0,nullptr);
    }
    mRebuildDescriptors=false;
  }

  void FastPipeline::WriteDescriptors(VkCommandBuffer CMDBuffer){
    if(mUseBuffer){
      std::vector<VkDeviceSize> layoutOffsets;
      std::vector<uint32_t> layoutIndecies;
      VkDeviceSize currentOffset=0;
      for(auto layout:mShaderObject->GetLayouts()){
        layoutOffsets.push_back(currentOffset);
        currentOffset+=layout->GetLayoutSize();
        layoutIndecies.push_back(0);
      }

      mDescriptorBuffer->CommandBufferBind(CMDBuffer);
      mLayout->SetDescriptorBufferOffsets(CMDBuffer,layoutIndecies,layoutOffsets);
    } else{

      VkShaderStageFlags stages=0;
      for(auto stage:mShaderObject->Stages())
        stages|=stage;

      VkBindDescriptorSetsInfo info{
        .sType=VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO,
        .pNext=nullptr,
        .stageFlags=stages,
        .layout=mLayout->Handle(),
        .firstSet=0,
        .descriptorSetCount=(uint32_t)mDescriptorSets.size(),
        .pDescriptorSets=mDescriptorSets.data(),
        .dynamicOffsetCount=0,
        .pDynamicOffsets=nullptr
      };
      vkCmdBindDescriptorSets2(CMDBuffer,&info);
    }
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
        mRebuildDescriptors=true;
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
    mRebuildDescriptors=true;
  }

  void FastPipeline::AssignImage(std::string name,BaseImagePtr image,ImageViewPtr view){
    mImages[name]=image;
    mImageViews[name]=view;
    mRebuildDescriptors=true;
  }
}
