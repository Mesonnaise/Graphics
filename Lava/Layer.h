#pragma once
#include<random>
#include<vector>
#include<array>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include<Device.h>
#include<FastComputePipeline.h>

class Layer{
protected:
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


  struct Configuration{
    glm::vec4 ClearColor;
    glm::vec2 Resolution;
    float Threshold;
    uint32_t LayerCount;
  };

  struct DistributionControl{
    std::uniform_real_distribution<float> XPosition;
    std::uniform_real_distribution<float> FloatRate;
    std::uniform_real_distribution<float> BlobSize;
    float MaxBlobSize;
  };

  Engine::FastComputePipelinePtr mPipeline;
  Configuration mConfiguration;
  std::vector<DistributionControl> mDistributions;
  std::uniform_real_distribution<float> mYSpread;
  std::ranlux48 mRandom;

protected:
  void GetVertexData(VertexData *&data);
public: 
  Layer(Engine::DevicePtr device,Engine::AllocatorPtr allocator);
  void ViewPort(uint32_t width,uint32_t height);
  void Color(std::vector<std::array<float,3>> LayerColor);
  void BlobCount(std::vector<uint32_t> BlobCounts);
  void FloatRate(uint32_t layerIndex,std::array<float,2> Rates);
  void BlobSize(uint32_t layerIndex,std::array<float,2> Size);

  void BackingImage(Engine::BaseImagePtr Image,Engine::ImageViewPtr View);
  void Pass(VkCommandBuffer CMDBuffer);

  void Step();
  glm::vec4 NewBlob(uint32_t layerIndex,bool spread=false);
};

