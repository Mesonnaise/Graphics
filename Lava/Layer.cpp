#include<filesystem>
#include<string>
#include<iostream>
#include<format>

#include "Layer.h"

//static std::filesystem::path ShaderPath="C:\\Users\\Mesonnaise\\Desktop\\Graphics\\x64\\Debug\\Blobs.spv";
static std::string ShaderName="Blobs.spv";

static uint32_t RoundUP(uint32_t value,uint32_t multi){
  return (value+multi-1)/multi;
}

Layer::Layer(Engine::DevicePtr device,Engine::AllocatorPtr allocator):
  mRandom(std::random_device{}()){

  std::filesystem::path ShaderPath=std::filesystem::current_path();
  ShaderPath.append(ShaderName);
  std::cout<<ShaderPath<<"\n";

  mPipeline=Engine::FastComputePipeline::Create(device,allocator,ShaderPath);
  mPipeline->QuickCreateBuffers();

  mConfiguration.Threshold=1.0f;
}

void Layer::ViewPort(uint32_t width,uint32_t height){
  mConfiguration.Resolution.x=width;
  mConfiguration.Resolution.y=height;
  
  float bottom=(float)height;
  float half=bottom/2.0f;

  mYSpread=std::uniform_real_distribution<float>(
    0.0-half,bottom+half);

  for(auto &dist:mDistributions)
    dist.XPosition=std::uniform_real_distribution<float>(0,(float)mConfiguration.Resolution.x);
}

void Layer::GetVertexData(VertexData *&data){
  data=reinterpret_cast<VertexData *>(mPipeline->GetBuffer("VertexData")->Mapped());
}

void Layer::Color(std::vector<std::array<float,3>> LayerColor){
  VertexData *data;
  GetVertexData(data);

  auto HsvToRgb=[](std::array<float,3> hsv)->glm::vec4{
    auto &[h,s,v]=hsv;
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

  mConfiguration.ClearColor=HsvToRgb(LayerColor[0]);

  if(LayerColor.size()>4)
    throw std::runtime_error("Only 4 Layers are Supported");

  for(int i=1;i<4;i++)
    data->BlobConfig[i-1].Color=HsvToRgb(LayerColor[i]);
}

void Layer::BlobCount(std::vector<uint32_t> Count){
  mConfiguration.LayerCount=(uint32_t)Count.size();
  VertexData *data;
  GetVertexData(data);

  mDistributions.clear();

  auto maxDim=std::max(mConfiguration.Resolution.x,mConfiguration.Resolution.y);

  for(size_t i=0;i<Count.size();i++)
    mDistributions.push_back({
      .XPosition=std::uniform_real_distribution<float>(0,(float)mConfiguration.Resolution.x),
      .FloatRate=std::uniform_real_distribution<float>(1.0,1.5),
      .BlobSize=std::uniform_real_distribution<float>(maxDim*0.04,maxDim*0.1),
      .MaxBlobSize=maxDim*0.1f
    });

  uint32_t offset=0;
  for(size_t i=0;i<Count.size();i++){
    auto blobCount=Count[i];
    data->BlobConfig[i].VertexCount=blobCount;
    data->BlobConfig[i].VertexOffset=offset;
    for(uint32_t c=0;c<blobCount;c++){
      data->Vertices[offset]=NewBlob(i,true);
      offset++;
    }
  }
}


void Layer::FloatRate(uint32_t layerIndex,std::array<float,2> Rates){
  if(layerIndex>=mConfiguration.LayerCount)
    throw std::runtime_error("Layer Index out of range");

  mDistributions[layerIndex].FloatRate=std::uniform_real_distribution<float>(Rates[0],Rates[1]);

  VertexData *data;
  GetVertexData(data);
  
  uint32_t offset=data->BlobConfig[layerIndex].VertexOffset;
  for(uint32_t i=0;i<data->BlobConfig[layerIndex].VertexCount;i++)
    data->Vertices[offset+i]=NewBlob(layerIndex,true);
}
void Layer::BlobSize(uint32_t layerIndex,std::array<float,2> Size){
  if(layerIndex>=mConfiguration.LayerCount)
    throw std::runtime_error("Layer Index out of range");

  mDistributions[layerIndex].BlobSize=std::uniform_real_distribution<float>(Size[0],Size[1]);
  mDistributions[layerIndex].MaxBlobSize=Size[1];

  VertexData *data;
  GetVertexData(data);

  uint32_t offset=data->BlobConfig[layerIndex].VertexOffset;
  for(uint32_t i=0;i<data->BlobConfig[layerIndex].VertexCount;i++)
    data->Vertices[offset+i]=NewBlob(layerIndex,true);
}

void Layer::BackingImage(Engine::BaseImagePtr Image,Engine::ImageViewPtr View){
  mPipeline->AssignImage("OutputImage",Image,View);
}

void Layer::Pass(VkCommandBuffer CMDBuffer){
  auto width=RoundUP(mConfiguration.Resolution.x,16);
  auto height=RoundUP(mConfiguration.Resolution.y,16);

  Step();
  mPipeline->AssignPush(&mConfiguration);
  mPipeline->PopulateCommandBuffer(CMDBuffer,width,height,1);
}

void Layer::Step(){
  VertexData *data;
  GetVertexData(data);
  
  float fieldTop=0.0f-(mConfiguration.Resolution.y/2.0f);

  for(uint32_t l=0;l<mConfiguration.LayerCount;l++){
    for(uint32_t i=0;i<data->BlobConfig[l].VertexCount;i++){
      auto index=i+data->BlobConfig[l].VertexOffset;
      data->Vertices[index].y-=data->Vertices[index].w;
      if((data->Vertices[index].y+data->Vertices[index].z)<fieldTop)
        data->Vertices[index]=NewBlob(l);
      
    }
  }
}

glm::vec4 Layer::NewBlob(uint32_t layerIndex,bool spread){
  glm::vec4 ret;
  //Horizontal position
  ret.x=mDistributions[layerIndex].XPosition(mRandom);
  //Blob radius
  ret.z=mDistributions[layerIndex].BlobSize(mRandom);
  //Vertical position
  if(spread)
    ret.y=mYSpread(mRandom);
  else
  ret.y=mConfiguration.Resolution.y*1.5+ret.z;
  //Speed of the blob
  ret.w=mDistributions[layerIndex].FloatRate(mRandom);
  return ret;
}