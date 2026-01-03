#pragma once
#include<filesystem>
#include<memory>
#include<map>
#include"Device.h"
#include"Allocator.h"
#include"ShaderObject.h"
#include"PipelineLayout.h"
#include"Buffer.h"
#include"Image.h"
#include"ImageView.h"

namespace Engine{
  class FastComputePipeline{
    DevicePtr mDevice=nullptr;
    AllocatorPtr mAllocator=nullptr;
    ShaderObjectPtr mShaderOjbect=nullptr;
    PipelineLayoutPtr mLayout=nullptr;

    bool mRebuildDescriptorBuffer=true;
    DescriptorBufferPtr mDescriptorBuffer=nullptr;

    std::map<std::string,BufferPtr> mBuffers;
    std::map<std::string,BaseImagePtr> mImages;
    std::map<std::string,ImageViewPtr> mImageViews;


    uint32_t mPushConstantSize=0;
    std::vector<uint8_t> mPushConstantData;

  protected:
    FastComputePipeline(DevicePtr &device,AllocatorPtr &allocator,std::filesystem::path &ShaderPath);

    void BuildDescriptorBuffer();

  public:
    static inline std::shared_ptr<FastComputePipeline> Create(DevicePtr &device,AllocatorPtr &allocator,std::filesystem::path ShaderPath){
      auto p=new FastComputePipeline(device,allocator,ShaderPath);
      return std::shared_ptr<FastComputePipeline>(p);
    }
    inline std::vector<std::string> GetVariableNames(){
      return mShaderOjbect->GetVariableNames();
    }

    inline BufferPtr GetBuffer(std::string name){
      if(mBuffers.find(name)==mBuffers.end())
        return nullptr;
      return mBuffers[name];
    }

    inline BaseImagePtr GetImage(std::string name){
      if(mImages.find(name)==mImages.end())
        return nullptr;
      return mImages[name];
    }

    inline void AssignBuffer(std::string name,BufferPtr buffer){
      mBuffers[name]=buffer;
      mRebuildDescriptorBuffer=true;
    }

    inline void AssignImage(std::string name,BaseImagePtr image,ImageViewPtr imageView){
      mImages[name]=image;
      mImageViews[name]=imageView;
      mRebuildDescriptorBuffer=true;
    }

    template<class T>
    inline void AssignPush(T *pushData){
      mPushConstantData.clear();
      mPushConstantData.resize(sizeof(T));
      *reinterpret_cast<T *>(mPushConstantData.data())=*pushData;
    }

    void QuickCreateBuffers();
    void PopulateCommandBuffer(VkCommandBuffer CMDBuffer,uint32_t x,uint32_t y,uint32_t z);

  };

  using FastComputePipelinePtr=std::shared_ptr<FastComputePipeline>;
}