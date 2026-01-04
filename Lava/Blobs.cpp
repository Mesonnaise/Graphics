#include<cinttypes>
#include<random>

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include "Blobs.h"

static const std::string ShaderName="Blobs.spv";

static constexpr uint32_t WorkGroupSize=16;
static constexpr uint32_t GLSLVertexCount=128;

struct VertexData{
  glm::vec4 Vertices[128];
};

struct PushConstantT{
  glm::vec4 Color[4];
  uint32_t Count[4];
};

static uint32_t RoundUP(uint32_t value){
  return (value+WorkGroupSize-1)/WorkGroupSize;
}

glm::vec4 Blobs::NewBlob(uint32_t layerIndex,bool spread){
  std::uniform_real_distribution<float> distCenter(-1.0f,1.0);
  auto &l=mLayers.at(layerIndex);
  glm::vec4 blob;

  blob.x=(float)mWidth*mDist(mRNG);
  blob.z=l.Radius+l.RadiusDeviation*distCenter(mRNG);
  blob.w=l.FloatSpeed+l.FloatSpeedDeviation*distCenter(mRNG);

  if(spread)
    blob.y=((float)mHeight*2.0*mDist(mRNG))-(float)mHeight*0.5;
  else
    blob.y=(float)mHeight*1.5f+blob.z;
  
  return blob;
}

Blobs::Blobs(Engine::DevicePtr &device,Engine::AllocatorPtr allocator){
  std::filesystem::path shaderPath=std::filesystem::current_path();
  shaderPath.append(ShaderName);
  mPipeline=Engine::FastComputePipeline::Create(device,allocator,shaderPath);
  mPipeline->QuickCreateBuffers();
}

void Blobs::BuildVertexCounts(){
  auto vertexdata=reinterpret_cast<VertexData *>(mPipeline->GetBuffer("VertexData")->Mapped());
  double renderArea=(double)(mWidth*mHeight);

  uint32_t totalVertexCount=0;
  for(size_t i=0;i<mLayers.size();i++){
    mLayers[i].VertexCount=(uint32_t)(renderArea*mLayers[i].Density);
    totalVertexCount+=mLayers[i].VertexCount;
  }

  if(totalVertexCount>=GLSLVertexCount){
    uint32_t theftCount=totalVertexCount-GLSLVertexCount;
    if(theftCount>mLayers.back().VertexCount)
      throw std::range_error("Unable to steal vertices from last layer");
    mLayers.back().VertexCount-=theftCount;
  }

  uint32_t vertexIndex=0;
  for(uint32_t layerIndex=0;layerIndex<mLayers.size();layerIndex++){
    auto &layer=mLayers[layerIndex];
    for(uint32_t vertexCount=0;vertexCount<layer.VertexCount;vertexCount++)
      vertexdata->Vertices[vertexIndex++]=NewBlob(layerIndex,true);
  }
}

void Blobs::SetColor(std::array<glm::vec3,4> Colors){
  auto HsvToRgb=[](glm::vec3 hsv)->glm::vec4{
    float h=hsv.x;
    float s=hsv.y;
    float v=hsv.z;

    float r,g,b;

    if(s==0.0f){
      return {v,v,v,1.0};
    }

    h/=60.0f; // sector 0 to 5
    int i=static_cast<int>(std::floor(h));
    float f=h-i; // factorial part of h
    float p=v*(1.0f-s);
    float q=v*(1.0f-s*f);
    float t=v*(1.0f-s*(1.0f-f));

    switch(i){
      case 0: r=v; g=t; b=p; break;
      case 1: r=q; g=v; b=p; break;
      case 2: r=p; g=v; b=t; break;
      case 3: r=p; g=q; b=v; break;
      case 4: r=t; g=p; b=v; break;
      default: r=v; g=p; b=q; break; // case 5
    }

    return {r,g,b,1.0};
  };

  mClearColor=HsvToRgb(Colors[0]);

  for(auto i=1;i<4;i++)
    mLayers[i-1].Color=HsvToRgb(Colors[i]);
}

void Blobs::SetBlobDensity(std::array<double,3> density){
  for(auto i=0;i<3;i++)
    mLayers[i].Density=density[i];

  BuildVertexCounts();
}

void Blobs::SetBlobRadius(std::array<float,3> radius,std::array<float,3> deviation){
  for(auto i=0;i<3;i++){
    mLayers[i].Radius=radius[i];
    mLayers[i].RadiusDeviation=deviation[i];
  }
}

void Blobs::SetBlobFloatSpeed(std::array<float,3> speed,std::array<float,3> deviation){
  for(auto i=0;i<3;i++){
    mLayers[i].FloatSpeed=speed[i];
    mLayers[i].FloatSpeedDeviation=deviation[i];
  }
}

void Blobs::SetBackingImage(Engine::BaseImagePtr image,Engine::ImageViewPtr view){
  mPipeline->AssignImage("OutputImage",image,view);

  auto extent=image->Extent();
  if(extent.width!=mWidth||extent.height!=mHeight){
    mWidth=extent.width;
    mHeight=extent.height;
    BuildVertexCounts();
  }
}

void Blobs::Step(VkCommandBuffer CMDBuffer,float delta){
  auto vertexdata=reinterpret_cast<VertexData *>(mPipeline->GetBuffer("VertexData")->Mapped());
  auto vertices=vertexdata->Vertices;

  PushConstantT pushData;
  pushData.Color[0]=mClearColor;

  uint32_t vertexOffset=0;
  for(uint32_t i=0;i<mLayers.size();i++){
    auto &layer=mLayers[i];

    pushData.Color[i+1]=layer.Color;
    pushData.Count[i]=layer.VertexCount;

    for(size_t vertexIndex=0;vertexIndex<layer.VertexCount;vertexIndex++){
      vertices[vertexOffset].y-=vertices[vertexOffset].w*delta;

      if(vertices[vertexOffset].y<((float)mHeight*-0.5f))
        vertices[vertexOffset]=NewBlob(i);

      vertexOffset++;
    }
  }

  mPipeline->AssignPush(&pushData);
  mPipeline->PopulateCommandBuffer(
    CMDBuffer,
    RoundUP(mWidth),RoundUP(mHeight),1);
}