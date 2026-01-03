#pragma once
#include<random>
#include<vector>
#include<array>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include<Device.h>
#include<FastComputePipeline.h>

class Blobs{
public:

  struct LayerConfig{
    float Density;
    float FloatSpeed;
    float FloatSpeedDeviation;
    float Radius;
    float RadiusDeviation;
  };




  uint32_t mWidth;
  uint32_t mHeight;
private:
  Engine::FastComputePipelinePtr mPipeline;
  std::default_random_engine mRNG;
  std::vector<LayerConfig> mLayers;
  
public:
  Blobs(Engine::DevicePtr &device,Engine::AllocatorPtr allocator);


  void Step(VkCommandBuffer CMDBuffer,double delta);
};

