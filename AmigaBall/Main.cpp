#include<filesystem>
#include<string>
#include<thread>
#include<chrono>
#include<iostream>
#include<format>
#include<cmath>
#include<numbers>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include<GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>


#include<Instance.h>
#include<Swapchain2.h>
#include<FastGraphicPipeline.h>
#include<FastComputePipeline.h>
#include<Allocator.h>
#include<CommandBufferBool.h>
#include<Mesh.h>

#include<glm/glm.hpp>
#include<glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"BallVertices.h"

using namespace std::chrono_literals;
const auto FrameTime=11ms;

const uint32_t ViewWidth=1280;
const uint32_t ViewHeight=720;

const std::string VertexShader="AmigaBallVertex.spv";
const std::string FragmentShader="AmigaBallFragment.spv";
const std::string ComputeShader="AmigaBallCompute.spv";

struct TransformMatrices{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
};

struct ShadowPush{
  glm::vec4 ShadowColor;
  glm::vec2 ShadowOffset;
};

struct Stencil{
  Engine::DevicePtr mDevice;
  Engine::AllocatorPtr mAllocator;
  Engine::BaseImagePtr mImage;
  Engine::ImageViewPtr mView;

  void CreateStencil(int width,int height){
    mImage=mAllocator->CreateImage(
      {(uint32_t)width,(uint32_t)height,1},
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);

    mView=mImage->CreateImageView();
  }
};


std::tuple<double,double> BallPosition(double t){
  double x=(std::abs(std::fmod(t*0.17+0.357,2.0)-1.0)-0.5)*5.0;
  double y=(std::abs(std::sin(t*std::numbers::pi*1.0))*-2.0)+1.0;

  return {x,y};
}

void SizeWindowCB(GLFWwindow *window,int width,int height){
  Stencil *StencilHandler=reinterpret_cast<Stencil *>(glfwGetWindowUserPointer(window));

  StencilHandler->CreateStencil(width,height);
}


int main(){
  glfwInit();

  uint32_t extBufferCount=0;
  const char **extBuffer=glfwGetRequiredInstanceExtensions(&extBufferCount);
  auto  inst=Engine::Instance::Create(std::vector<const char *>(extBuffer,extBuffer+extBufferCount));

  GLFWwindow *window=glfwCreateWindow(ViewWidth,ViewHeight,"Ball",NULL,NULL);

  if(!window){
    glfwTerminate();
    return -1;
  }

  glfwSetWindowSizeCallback(window,SizeWindowCB);
  glfwSetWindowPos(window,1750,600);

  std::filesystem::path workingDir=std::filesystem::current_path();
  auto vertexPath=workingDir;
  auto fragmentPath=workingDir;
  auto computePath=workingDir;

  vertexPath.append(VertexShader);
  fragmentPath.append(FragmentShader);
  computePath.append(ComputeShader);

  auto phy=inst->EnumeratePhysicals()[0];
  auto surface=inst->CreateSurface(window);
  auto device=inst->CreateDevice(phy,surface);
  auto swapchain=Engine::Swapchain2::Create(device,surface);
  auto allocator=Engine::Allocator::Create(inst,device);

  auto queue=device->GetQueue(0);

  Stencil stencil;
  stencil.mDevice=device;
  stencil.mAllocator=allocator;
  stencil.CreateStencil(ViewWidth,ViewHeight);
  glfwSetWindowUserPointer(window,&stencil);

  auto ballPipeline=Engine::FastGraphicPipeline::Create(device,allocator,{vertexPath,fragmentPath},true);
  ballPipeline->QuickCreateBuffers();

  auto dropShadowPipeline=Engine::FastComputePipeline::Create(device,allocator,computePath);

  ShadowPush shadowPush={
    .ShadowColor={0.0f,0.0f,0.0f,0.5f},
    .ShadowOffset={0.1f,0.1f}
  };


  auto transforms=static_cast<TransformMatrices *>(ballPipeline->GetBuffer("Matrices")->Mapped());
  transforms->model=glm::mat4(1.0f);
  transforms->view=glm::lookAt(
    glm::vec3(0.0f,0.0f,-5.0f),
    glm::vec3(0.0f,0.0f,0.0f),
    glm::vec3(0.0f,-1.0f,0.0f));
  transforms->projection=glm::perspective(
    glm::radians(60.0f),
    (float)ViewWidth/(float)ViewHeight,
    0.1f,100.0f);

  auto vertexBuffer=allocator->CreateBuffer(
    BallVertices.size()*sizeof(glm::vec3),
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    true);

  auto vertexData=static_cast<glm::vec4 *>(vertexBuffer->Mapped());
  memcpy(vertexData,BallVertices.data(),sizeof(glm::vec3)*BallVertices.size());
  ballPipeline->AssignVertexBuffer(vertexBuffer);
  
  auto cmdPool=Engine::CommandBufferBool::Create(device,queue->QueueFamily());
  auto CMDBuffer=cmdPool->AllocateBuffers(1)[0];
  double step=0.0;

  while(!glfwWindowShouldClose(window)){
    const auto startTime=std::chrono::high_resolution_clock::now();
    auto [swapImage,swapView]=swapchain->Next();

    step+=0.01f;

    auto [ballX,ballY]=BallPosition(step);

    auto rotation=std::fmodf(step,std::numbers::pi*2.0f);
    transforms->model=glm::translate(glm::mat4(1.0f),{ballX,ballY,0.0f});
    transforms->model=glm::rotate(transforms->model,0.52f,{0,0.0,1.0});
    transforms->model=glm::rotate(transforms->model,rotation,{0,1,0});
    
    auto swapExtent=swapImage->Extent();
    transforms->projection=glm::perspective(
      glm::radians(60.0f),
      (float)swapExtent.width/(float)swapExtent.height,
      0.1f,100.0f);

    ballPipeline->ClearAttachments();
    
    VkRenderingAttachmentInfo attach=swapView->BasicAttachment(VK_IMAGE_LAYOUT_GENERAL);
    attach.clearValue.color={0.85,0.85,0.85,1.0};
    ballPipeline->AddAttachment(swapView,attach);
    //ballPipeline->AddDepthAttachment(stencil.mView);
    ballPipeline->AddStencilAttachment(stencil.mView);
    ballPipeline->SetViewport(swapImage->Extent());

    //dropShadowPipeline->AssignImage("OutputImage",swapImage,swapView);
    //dropShadowPipeline->AssignImage("ShadowImage",stencil.mImage,stencil.mView);
    //dropShadowPipeline->AssignPush(&shadowPush);

    VkCommandBufferBeginInfo info={
      .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext=nullptr,
      .flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo=nullptr
    };

    //******************* Start Command Buffer **************************

    vkBeginCommandBuffer(CMDBuffer,&info);
    stencil.mImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_NONE,0,
      VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
      VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );


    swapImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_NONE,0,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      VK_IMAGE_LAYOUT_GENERAL);



    ballPipeline->PopulateCommandBuffer(CMDBuffer,(uint32_t)BallVertices.size(),1,0,0);

    /*swapImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT|VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
      VK_IMAGE_LAYOUT_GENERAL);
      
    stencil.mImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
      VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
      VK_IMAGE_LAYOUT_GENERAL
    );*/

    /*dropShadowPipeline->PopulateCommandBuffer(
      CMDBuffer,
      (uint32_t)std::ceil(swapExtent.width/16.0f),
      (uint32_t)std::ceil(swapExtent.height/16.0f),
      1);
      
    swapImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
      VK_PIPELINE_STAGE_2_NONE,
      VK_ACCESS_2_NONE,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
      */
    swapImage->TransitionLayout(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_2_NONE,
      VK_ACCESS_2_NONE,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //vkCmdEndRenderPass(CMDBuffer);
    vkEndCommandBuffer(CMDBuffer);
    queue->Submit({CMDBuffer});
    queue->Wait();
    swapchain->PresentImage(queue,swapImage);


    const auto endTime=std::chrono::high_resolution_clock::now();
    auto elapsed=endTime-startTime;
    std::this_thread::sleep_for(FrameTime-elapsed);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}