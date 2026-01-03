#include<cinttypes>
#include "Blobs.h"

static const uint32_t WorkGroupSize=16;


struct BlobLayerConfig{
  glm::vec4 Color;
  uint32_t VertexOffset;
  uint32_t VertexCount;
  glm::vec2 Padding;
};

struct VertexData{
  BlobLayerConfig BlobConfig[4];
  glm::vec4 Vertices[128];
};

struct PushConstantT{
  glm::vec3 ClearColor;
  glm::vec3 Color1;
  glm::vec3 Color2;
  glm::vec3 Color3;
  uint32_t Count1;
  uint32_t Count2;
  uint32_t Count3;
  uint32_t Padding;
};

static uint32_t RoundUP(uint32_t value){
  return (value+WorkGroupSize-1)/WorkGroupSize;
}



void Blobs::Step(VkCommandBuffer CMDBuffer,double delta){
  auto vertexdata=reinterpret_cast<VertexData *>(mPipeline->GetBuffer("VertexData")->Mapped());
  PushConstantT pushData;

  mPipeline->AssignPush(&pushData);
  mPipeline->PopulateCommandBuffer(
    CMDBuffer,
    RoundUP(mWidth),RoundUP(mHeight),1);


}