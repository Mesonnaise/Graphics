#include "FastComputePipeline.h"


FastComputePipeline::FastComputePipeline(DevicePtr &device,AllocatorPtr &allocator,std::filesystem::path &ShaderPath):
  mDevice(device),mAllocator(allocator){

  mShaderOjbect=ShaderObject::Create(mDevice,{ShaderPath});
  mLayout=PipelineLayout::Create(mDevice,{mShaderOjbect});
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

    }else if(descriptorType==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
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

  vkCmdDispatch(CMDBuffer,x,y,z);
}