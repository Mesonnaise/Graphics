#include<iostream>
#include<format>
#include<filesystem>
#include<random>
#include<thread>
#include<array>
#include<vector>
#include<cmath>


#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include<GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>

#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include<Instance.h>
#include<Device.h>
#include<Allocator.h>
#include<CommandBufferBool.h>
#include<FastComputePipeline.h>
#include<Swapchain2.h>
//#include<Swapchain.h>
#include"Layer.h"

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

std::vector<std::array<float,3>> ColorMapping={
  {247.13,1.00,0.65},
  {255.10,0.6942,0.8078},
  {268.90,0.6431,1.00},
  {278.97,0.4795,0.9669}
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



void SetColor(Layer *layer){
  auto palletRNG=std::random_device();
  auto dist=std::uniform_real_distribution<float>(0,360.0f);
  float offset=dist(palletRNG);
  
  for(int i=0;i<4;i++){
    ColorMapping[i][0]=std::fmod(ColorMapping[i][0]+offset,360.0f);
    std::cout<<std::format("{},{},{}\n",ColorMapping[i][0],ColorMapping[i][1],ColorMapping[i][2]);
  }
  std::cout<<"\n";
  layer->Color(ColorMapping);
}


static void SetBlobParams(Layer *layer){
  float speedScale=2.0f;
  auto area=ViewWidth*ViewHeight;

  layer->BlobCount({ViewWidth/160,ViewWidth/80,ViewWidth/60});
  layer->BlobSize(0,{172.8,230.4});
  layer->FloatRate(0,{1.0f*speedScale,1.5f*speedScale});

  layer->BlobSize(1,{57.6,153.6});
  layer->FloatRate(1,{1.0f*speedScale,2.5f*speedScale});

  layer->BlobSize(2,{19.2,96});
  layer->FloatRate(2,{1.75f*speedScale,3.0f*speedScale});
}


void ErrorCallbackGLFW(int error,const char *description){
  std::cout<<std::format("{}\n",description);
}


void CloseWindowCB(GLFWwindow *window){
  std::cout<<"Window Closing\n";
}

void KeyWindowCB(GLFWwindow *window,int key,int scancode,int action,int mods){


  if(key==32&&action==1){
    Layer *blobHandler=reinterpret_cast<Layer *>(glfwGetWindowUserPointer(window));
    SetColor(blobHandler);
  }else if(key!=32)
    std::cout<<std::format(
      "key {} scancode {} action {} mods {}\n",
      key,scancode,action,mods);
}



void SizeWindowCB(GLFWwindow *window,int width,int height){
  Layer *blobHandler=reinterpret_cast<Layer *>(glfwGetWindowUserPointer(window));
  blobHandler->ViewPort(width,height);
  //std::cout<<std::format("Window {} {}\n",width,height);
  ViewWidth=width;
  ViewHeight=height;
  
  SetBlobParams(blobHandler);

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
  glfwInit();
  uint32_t extBufferCount=0;
  const char **extBuffer=glfwGetRequiredInstanceExtensions(&extBufferCount);
  auto  inst=Engine::Instance::Create(std::vector<const char *>(extBuffer,extBuffer+extBufferCount));

  glfwSetErrorCallback(ErrorCallbackGLFW);

  GLFWwindow *window=glfwCreateWindow(ViewWidth,ViewHeight,"Window Title",NULL,NULL);
  if(!window){
    glfwTerminate();
    return -1;
  }

  glfwSetWindowCloseCallback(window,CloseWindowCB);
  glfwSetKeyCallback(window,KeyWindowCB);
  glfwSetWindowSizeCallback(window,SizeWindowCB);

  auto phy=inst->EnumeratePhysicals()[0];
  auto surface=inst->CreateSurface(window);
  auto device=inst->CreateDevice(phy,surface);
  auto swapchain=Engine::Swapchain2::Create(device,surface);

  auto allocator=Engine::Allocator::Create(inst,device);

  auto queue=device->GetQueue(0);
  auto cmdPool=Engine::CommandBufferBool::Create(device,queue->QueueFamily());
  auto CMDBuffer=cmdPool->AllocateBuffers(1)[0];

  Layer layer(device,allocator);
  glfwSetWindowUserPointer(window,&layer);


  layer.ViewPort(ViewWidth,ViewHeight);
  //layer.BlobCount({12,24,32});
  SetBlobParams(&layer);
  SetColor(&layer);
  

  //float speedScale=2.0f;

  //layer.BlobSize(0,{ViewWidth*0.09f,ViewWidth*0.12f});
  //layer.FloatRate(0,{1.0f*speedScale,1.5f*speedScale});

  //layer.BlobSize(1,{ViewWidth*0.03f,ViewWidth*0.08f});
  //layer.FloatRate(1,{1.0f*speedScale,2.5f*speedScale});

  //layer.BlobSize(2,{ViewWidth*0.01f,ViewWidth*0.05f});
  //layer.FloatRate(2,{1.75f*speedScale,3.0f*speedScale});

  while(!glfwWindowShouldClose(window)){
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

    layer.BackingImage(swapImage,swapView);
    layer.Pass(CMDBuffer);

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