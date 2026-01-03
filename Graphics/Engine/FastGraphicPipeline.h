#pragma once
#include<memory>
#include<filesystem>
#include"Device.h"
#include"Allocator.h"
#include"ShaderObject.h"
#include"PipelineLayout.h"
#include"Buffer.h"
#include"Image.h"
#include"ImageView.h"

class FastGraphicPipeline{
  DevicePtr mDevice=nullptr;
  AllocatorPtr mAllocator=nullptr;
  ShaderObjectPtr mShaderOjbect=nullptr;
  PipelineLayoutPtr mLayout=nullptr;

  bool mRebuildDescriptorBuffer=true;
  DescriptorBufferPtr mDescriptorBuffer=nullptr;

  std::map<std::string,BufferPtr> mBuffers;
  std::map<std::string,ImagePtr> mImages;
  std::map<std::string,ImageViewPtr> mImageViews;

  BufferPtr mVertexBuffer=nullptr;
  ImagePtr mOutputImage=nullptr;
  ImageViewPtr mOutputImageView=nullptr;

protected:
  FastGraphicPipeline(DevicePtr device,AllocatorPtr allocator,std::vector<std::filesystem::path> shaders);

  void BuildDescriptorBuffer();
public:
  static inline std::shared_ptr<FastGraphicPipeline> Create(DevicePtr device,AllocatorPtr allocator,std::vector<std::filesystem::path> shaders){
    auto p=new FastGraphicPipeline(device,allocator,shaders);
    return std::shared_ptr<FastGraphicPipeline>(p);
  }
  inline std::vector<std::string> GetVariableNames(){
    return mShaderOjbect->GetVariableNames();
  }
  inline BufferPtr GetBuffer(std::string name){
    if(mBuffers.find(name)==mBuffers.end())
      return nullptr;
    return mBuffers[name];
  }

  inline ImagePtr GetImage(std::string name){
    if(mImages.find(name)==mImages.end())
      return nullptr;
    return mImages[name];
  }

  inline void AssignBuffer(std::string name,BufferPtr buffer){
    mBuffers[name]=buffer;
  }
  inline void AssignImage(std::string name,ImagePtr image){
    mImages[name]=image;
  }
  void QuickCreateBuffers();
  void PopulateCommandBuffer(
    VkCommandBuffer CMDBuffer,
    uint32_t vertexCount,uint32_t instanceCount,
    uint32_t firstVertex,uint32_t firstInstance);

};

using FastGraphicPipelinePtr=std::shared_ptr<FastGraphicPipeline>;

