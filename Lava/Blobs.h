#pragma once
#include<random>
#include<vector>
#include<array>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include<Device.h>
#include<FastComputePipeline.h>
#include<Image.h>
#include<ImageView.h>

class Blobs{
private:

  struct LayerConfig{
    double Density=0.01;
    float FloatSpeed=0.05f;
    float FloatSpeedDeviation=0.01f;
    float Radius=25.0f;
    float RadiusDeviation=0.1f;
    glm::vec4 Color={0.5f,0.5f,0.5f,1.0f};
    uint32_t VertexCount=6;
  };




  uint32_t mWidth=0;
  uint32_t mHeight=0;
  glm::vec4 mClearColor={0.0f,0.0f,0.0f,1.0f};
  std::array<LayerConfig,3> mLayers;

  Engine::FastComputePipelinePtr mPipeline;
  std::default_random_engine mRNG;
  std::uniform_real_distribution<float> mDist;
  
private:
  void BuildVertexCounts();
  glm::vec4 NewBlob(uint32_t layerIndex,bool spread=false);
public:
  Blobs(Engine::DevicePtr &device,Engine::AllocatorPtr allocator);
  void SetColor(std::array<glm::vec3,4> Colors);
  void SetBlobDensity(std::array<double,3> density);
  void SetBlobRadius(std::array<float,3> radius,std::array<float,3> deviation);
  void SetBlobFloatSpeed(std::array<float,3> speed,std::array<float,3> deviation);
  void SetBackingImage(Engine::BaseImagePtr image,Engine::ImageViewPtr view);
  void Step(VkCommandBuffer CMDBuffer,float delta);
};

