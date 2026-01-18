#pragma once
#include<memory>
#include<string>
#include<vector>
#include<map>
#include<filesystem>

#include<vulkan/vulkan.h>

#include"Allocator.h"
#include"ShaderObject.h"
#include"PipelineLayout.h"
#include"Buffer.h"
#include"Image.h"
#include"ImageView.h"
#include"DescriptorPool.h"

namespace Engine{
  class FastPipeline{
  protected:
    DevicePtr mDevice=nullptr;
    AllocatorPtr mAllocator=nullptr;
    ShaderObjectPtr mShaderObject=nullptr;
    PipelineLayoutPtr mLayout=nullptr;

    bool mUseBuffer=false;
    bool mRebuildDescriptors=false;
    DescriptorBufferPtr mDescriptorBuffer=nullptr;

    std::map<std::string,BufferPtr> mBuffers;
    std::map<std::string,BaseImagePtr> mImages;
    std::map<std::string,ImageViewPtr> mImageViews;

    uint32_t mPushConstantSize=0;
    std::vector<uint8_t> mPushConstantData;

    DescriptorPoolPtr mDescriptorPool=nullptr;
    std::vector<VkDescriptorSet> mDescriptorSets;
  protected:
    FastPipeline(DevicePtr &device,AllocatorPtr &allocator,std::vector<std::filesystem::path> shaderPaths);
    void UpdateDescriptors();
    void WriteDescriptors(VkCommandBuffer CMDBuffer);
  public:
    template<class T>
    inline void AssignPush(T *pushData){
      mPushConstantData.clear();
      mPushConstantData.resize(sizeof(T));
      *reinterpret_cast<T *>(mPushConstantData.data())=*pushData;
    }

    void QuickCreateBuffers();
    std::vector<std::string> GetVariableNames();
    BufferPtr GetBuffer(std::string name);
    BaseImagePtr GetImage(std::string name);

    void AssignBuffer(std::string name,BufferPtr buffer);
    void AssignImage(std::string name,BaseImagePtr image,ImageViewPtr view);

   // virtual void BuildCommandBuffer(VkCommandBuffer CMDBuffer)=0;
  };
}