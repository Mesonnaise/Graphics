#include<iostream>
#include<format>
#include<filesystem>
#include<random>
#include<thread>
#include<array>
#include<vector>
#include<cmath>

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include<Instance.h>
#include<Device.h>
#include<Allocator.h>
#include<CommandBufferBool.h>
#include<Swapchain2.h>
//#include<Swapchain.h>
#include"Blobs.h"

using namespace std::chrono_literals;
using CMappingT=std::vector<std::array<float,3>>;

static uint32_t ViewWidth=1920;
static uint32_t ViewHeight=1080;

//247.13,1.00,0.65
//255.10,0.6942,0.8078
//268.90,0.6431,1.00
//278.97,0.4795,0.9669

//231.0033,1,0.65
//238.9733,0.6942,0.8078
//252.77328,0.6431,1
//262.84323,0.4795,0.9669

//15.791382,1,0.65
//23.761353,0.6942,0.8078
//37.561356,0.6431,1
//47.6313,0.4795,0.9669

//352.91293,1,0.65
//0.8829651,0.6942,0.8078
//14.682953,0.6431,1
//24.7529,0.4795,0.9669

//15.791382,1,0.65
//23.761353,0.6942,0.8078
//37.561356,0.6431,1
//47.6313,0.4795,0.9669

std::array<glm::vec3,4> ColorMapping={
  glm::vec3{247.13,1.00,0.65},
  glm::vec3{255.10,0.6942,0.8078},
  glm::vec3{268.90,0.6431,1.00},
  glm::vec3{278.97,0.4795,0.9669}
};

static const std::vector<std::array<uint32_t,3>> ColorPalettes={
  {19,0,160},
  {99,63,206},
  {170,91,255},
  {203,127,244},

  {77,50,6},
  {127,91,31},
  {186,155,71},
  {224,222,127}
};



void SetColor(Blobs *blobHandler){
  auto palletRNG=std::random_device();
  auto dist=std::uniform_real_distribution<float>(0,360.0f);
  float offset=dist(palletRNG);
  
  for(int i=0;i<4;i++){
    ColorMapping[i].x=std::fmod(ColorMapping[i].x+offset,360.0f);
    std::cout<<std::format("{},{},{}\n",ColorMapping[i].x,ColorMapping[i].y,ColorMapping[i].z);
  }
  std::cout<<"\n";
  blobHandler->SetColor(ColorMapping);
}

void ErrorCallbackGLFW(int error,const char *description){
  std::cout<<std::format("{}\n",description);
}

const auto FrameTime=11ms;

static glm::vec4 ConvertColor(std::array<uint32_t,3> color){
  const float s=256.0f;
  const float tr=(float)color[0];
  const float tg=(float)color[1];
  const float tb=(float)color[2];
  return glm::vec4(tr/s,tg/s,tb/s,1.0f);
}


int main(){
  using namespace std::chrono_literals;
  auto  inst=Engine::Instance::Create({},true,true);

  glfwSetErrorCallback(ErrorCallbackGLFW);

  auto window=inst->CreateWindow(ViewWidth,ViewHeight,"Lava");

  auto phy=inst->EnumeratePhysicals()[0];

  auto device=Engine::Device::Create(inst,phy,window);
  auto swapchain=Engine::Swapchain2::Create(device,window);

  auto allocator=Engine::Allocator::Create(inst,device);

  auto queue=device->GetQueue(0);
  auto cmdPool=Engine::CommandBufferBool::Create(device,queue->QueueFamily());
  auto CMDBuffer=cmdPool->AllocateBuffers(1)[0];

  Blobs blobs(device,allocator);
  SetColor(&blobs);
  blobs.SetBlobDensity({0.00000482,0.00000965,0.00001546});
  blobs.SetBlobRadius({200,105,60},{20.0,30,15});
  blobs.SetBlobFloatSpeed({0.10,0.2,0.35},{0.075,0.05,0.05});

  window->KeyCallback([&blobs](int key,int scancode,int action,int mods){
    if(key==32&&action==1){
      SetColor(&blobs);
    }
  });

  while(!window->ShouldClose()){
    const auto startTime=std::chrono::high_resolution_clock::now();
    
    auto [swapImage,swapView]=swapchain->Next();

    VkCommandBufferBeginInfo info={
      .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext=nullptr,
      .flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo=nullptr
    };

    vkBeginCommandBuffer(CMDBuffer,&info);
    swapImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_NONE,0,
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
      VK_IMAGE_LAYOUT_GENERAL);

    blobs.SetBackingImage(swapImage,swapView);
    blobs.Step(CMDBuffer,11.0);

    swapImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
      VK_PIPELINE_STAGE_2_NONE,
      VK_ACCESS_2_NONE,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(CMDBuffer);

    queue->Submit({CMDBuffer});
    queue->Wait();

    swapchain->PresentImage(queue,swapImage);
    const auto endTime=std::chrono::high_resolution_clock::now();
    auto elapsed=endTime-startTime;
    std::this_thread::sleep_for(FrameTime-elapsed);
    glfwPollEvents();
  }
}