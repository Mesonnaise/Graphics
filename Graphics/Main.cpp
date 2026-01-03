/*
  The arguments -e and --sep are needed to ranme the entry point 
  glslangValidator.exe -e VertMain  -V -S <stage> .\vert.glsl --sep main -o ..\x64\Debug\vert.spv


*/
#define NOMINMAX
#include<iostream>
#include<format>
#include<vector>
#include<filesystem>
#include<cmath>
#include<algorithm>
#include<numbers>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include<GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>

#include<glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"Engine\Instance.h"
#include"Engine\Device.h"
#include"Engine\Swapchain.h"
#include"Engine\CommandBufferBool.h"
#include"Engine\ShaderObject.h"
#include"Engine\PipelineLayout.h"
#include"Engine\Util.h"
#include"Engine\Allocator.h"

#include"Engine\FastComputePipeline.h"

void ErrorCallbackGLFW(int error,const char *description){
  std::cout<<std::format("{}\n",description);
}


void CloseWindowCB(GLFWwindow *window){
  std::cout<<"Window Closing\n";
}

void KeyWindowCB(GLFWwindow *window,int key,int scancode,int action,int mods){
  std::cout<<std::format("key {} scancode {} action {} mods {}\n",
    key,scancode,action,mods);
}

void SizeWindowCB(GLFWwindow *window,int width,int height){
  std::cout<<std::format("Window {} {}\n",width,height);
}


struct VertexProjection{
  glm::mat4 modelMatrix;
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
};

const std::filesystem::path vertPath="C:\\Users\\Mesonnaise\\Desktop\\Graphics\\x64\\Debug\\testVertex.spv";
const std::filesystem::path fragPath="C:\\Users\\Mesonnaise\\Desktop\\Graphics\\x64\\Debug\\testFragment.spv";
const std::filesystem::path compPath="C:\\Users\\Mesonnaise\\Desktop\\Graphics\\x64\\Debug\\comp.spv";


const uint32_t ViewWidth=640;
const uint32_t ViewHeight=480;

int main(){
  glfwInit();
  uint32_t extBufferCount=0;
  const char **extBuffer=glfwGetRequiredInstanceExtensions(&extBufferCount);
  auto  inst=Instance::Create(std::vector<const char *>(extBuffer,extBuffer+extBufferCount));

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
  SurfacePtr surface=inst->CreateSurface(window);
  DevicePtr device=inst->CreateDevice(phy,surface);
  LoadSymbols(device->Handle());

  auto swapchain=device->CreateSwapchain(surface);

  auto allocator=Allocator::Create(inst,device);

  auto computePipeline=FastComputePipeline::Create(device,allocator,compPath);
  computePipeline->QuickCreateBuffers();

  auto queue=device->GetQueue(0);
  CommandBufferBoolPtr cmdPool=CommandBufferBool::Create(device,queue->QueueFamily());
  auto CMDBuffer=cmdPool->AllocateBuffers(1)[0];

    
  double tick=0.0;
  auto blend=[&tick](){
    const double tau=std::numbers::pi*2.0;
    const double zeroPoint=0.0;
    const double onethirdPoint=tau*1.0/3.0;
    const double twoThirdPoint=tau*2.0/3.0;

    auto tmp=std::fmod(tick,tau);
    auto v1=std::sin(tmp+zeroPoint)*0.5+0.5;
    auto v2=std::sin(tmp+onethirdPoint)*0.5+0.5;
    auto v3=std::sin(tmp+twoThirdPoint)*0.5+0.5;
    tick+=0.001;
    return glm::vec4(v1,v2,v3,1.0);
  };
  

  while(!glfwWindowShouldClose(window)){
    auto mappedData=computePipeline->GetBuffer("COLOR")->Mapped();
    *reinterpret_cast<glm::vec4 *>(mappedData)=blend();

    auto imageIndex=swapchain->AcquireNextImage();

    VkCommandBufferBeginInfo info={
      .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext=nullptr,
      .flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo=nullptr
    };

    vkBeginCommandBuffer(CMDBuffer,&info);

    swapchain->TransitionSwapchain(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_NONE,0,
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
      VK_IMAGE_LAYOUT_GENERAL);

    computePipeline->AssignImage("OutputImage",
      swapchain->GetImage(imageIndex),swapchain->GetImageView(imageIndex));


    auto swapImageSize=swapchain->GetImage(imageIndex)->Extent();
    computePipeline->PopulateCommandBuffer(CMDBuffer,swapImageSize.width,swapImageSize.height,1);


    swapchain->TransitionSwapchain(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
      VK_PIPELINE_STAGE_2_NONE,
      VK_ACCESS_2_NONE,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);


    vkEndCommandBuffer(CMDBuffer);
    queue->Submit({CMDBuffer});
    queue->Wait();

    swapchain->PresentImage(queue);
    glfwPollEvents();
  }

  return 0;
}

int main2(){
  glfwInit();
  uint32_t extBufferCount=0;
  const char **extBuffer=glfwGetRequiredInstanceExtensions(&extBufferCount);
  auto  inst=Instance::Create(std::vector<const char *>(extBuffer,extBuffer+extBufferCount));

  glfwSetErrorCallback(ErrorCallbackGLFW);


  if(glfwVulkanSupported())
    std::cout<<"Vulkan supported\n";
  else
    std::cout<<"Vulkan not supported\n";

  GLFWwindow *window=glfwCreateWindow(ViewWidth,ViewHeight,"Window Title",NULL,NULL);
  if(!window){
    glfwTerminate();
    return -1;
  }

  glfwSetWindowCloseCallback(window,CloseWindowCB);
  glfwSetKeyCallback(window,KeyWindowCB);
  glfwSetWindowSizeCallback(window,SizeWindowCB);

  auto phy=inst->EnumeratePhysicals()[0];
  SurfacePtr surface=inst->CreateSurface(window);
  DevicePtr device=inst->CreateDevice(phy,surface);

  LoadSymbols(device->Handle());
  auto allocator=Allocator::Create(inst,device);

  auto shader=ShaderObject::Create(device,{vertPath,fragPath});

  //std::cout<<shader->DumpInfo();

  auto pipelineLayout=PipelineLayout::Create(device,{shader});

  auto vertexBuffer=allocator->CreateBuffer(6*sizeof(glm::vec3),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,true);
  auto sceneBuffer=allocator->CreateBuffer(shader->VariableStructureSize("SceneData"),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,true);

  auto descriptorBuffer=allocator->CreateDescriptorBuffer(shader->GetDescriptorBufferTotalSize()+1024,true);

  sceneBuffer->GetDescriptor(descriptorBuffer->Mapped(),VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

  auto *projection=reinterpret_cast<VertexProjection *>(sceneBuffer->Mapped());
  auto *vertex=reinterpret_cast<glm::vec3 *>(vertexBuffer->Mapped());

  projection->modelMatrix=glm::mat4(1.0f);
  projection->viewMatrix=glm::lookAt(
    glm::vec3( 0.0f, 0.0f,-1.0f),
    glm::vec3( 0.0f, 0.0f, 0.0f),
    glm::vec3( 0.0f, 1.0f, 0.0f));
  projection->projectionMatrix=glm::perspective(
    glm::radians(90.0f),
    (float)ViewWidth/(float)ViewHeight,
    0.1f,100.0f);


  vertex[0]=glm::vec3(-1.0f,1.0f,0.0f);
  vertex[1]=glm::vec3(1.0f,1.0f,0.0f);
  vertex[2]=glm::vec3(-1.0f,-1.0f,0.0f);


  vertex[3]=glm::vec3(1.0f,1.0f,0.0f);
  vertex[4]=glm::vec3(1.0f,-1.0f,0.0f);
  vertex[5]=glm::vec3(-1.0f,-1.0f,0.0f);

  auto queue=device->GetQueue(0);

  CommandBufferBoolPtr cmdPool=CommandBufferBool::Create(device,queue->QueueFamily());
  SwapchainPtr swapchain=device->CreateSwapchain(surface);


 
  auto CMDBuffer=cmdPool->AllocateBuffers(1)[0];



  std::vector<VkBuffer> vertexBufferHandles={vertexBuffer->Handle()};
  std::vector<VkDeviceSize> vertexBufferOffsets={0};

  VkViewport viewPort={
    .x=0.0f,
    .y=0.0f,
    .width=(float)ViewWidth,
    .height=(float)ViewHeight,
    .minDepth=0.0f,
    .maxDepth=1.0f
  };

  VkVertexInputBindingDescription2EXT vertexInputBinding={
    .sType=VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
    .pNext=nullptr,
    .binding=0,
    .stride=sizeof(glm::vec3),
    .inputRate=VK_VERTEX_INPUT_RATE_VERTEX,
    .divisor=1
  };

  VkVertexInputAttributeDescription2EXT vertexInputAttribute{
    .sType=VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
    .pNext=nullptr,
    .location=0,
    .binding=0,
    .format=VK_FORMAT_R32G32B32_SFLOAT,
    .offset=0
  };

  VkBool32 vkFalse=VK_FALSE;
  VkColorComponentFlags MaskColorComponent=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
  VkSampleMask SampleMask=0x1;

  while(!glfwWindowShouldClose(window)){


    auto imageIndex=swapchain->AcquireNextImage();

    VkCommandBufferBeginInfo beginInfo={
      .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext=nullptr,
      .flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo=nullptr
    };
    vkBeginCommandBuffer(CMDBuffer,&beginInfo);
    swapchain->TransitionSwapchain(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_NONE,0,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      VK_IMAGE_LAYOUT_GENERAL);

    VkRenderingAttachmentInfo attachmentInfo={
        .sType=VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext=nullptr,
        .imageView=swapchain->GetImageView(imageIndex)->Handle(),
        .imageLayout=VK_IMAGE_LAYOUT_GENERAL,
        .resolveMode=VK_RESOLVE_MODE_NONE,
        .resolveImageView=VK_NULL_HANDLE,
        .resolveImageLayout=VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp=VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue={.color={1.0f,0.0f,0.0f,1.0f}}
    };
    VkRect2D scissor={
      .offset={0,0},
      .extent={ViewWidth,ViewHeight}
    };
    VkRenderingInfo renderingInfo={
      .sType=VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext=nullptr,
      .flags=0,
      .renderArea={
        .offset={0,0},
        .extent={ViewWidth,ViewHeight}
      },
      .layerCount=1,
      .viewMask=0,
      .colorAttachmentCount=1,
      .pColorAttachments=&attachmentInfo,
    };

    vkCmdBeginRendering(CMDBuffer,&renderingInfo);
    BasicGraphicsPipeline(CMDBuffer);

    vkCmdSetViewportWithCount(CMDBuffer,1,&viewPort);
    vkCmdSetScissorWithCount(CMDBuffer,1,&scissor);

    pfnCmdSetVertexInputEXT(CMDBuffer,1,&vertexInputBinding,1,&vertexInputAttribute);

    pipelineLayout->BindShaders(CMDBuffer);

    vkCmdBindVertexBuffers(CMDBuffer,0,1,vertexBufferHandles.data(),vertexBufferOffsets.data());
    descriptorBuffer->CommandBufferBind(CMDBuffer);
    pipelineLayout->SetDescriptorBufferOffsets(CMDBuffer,{0},{0});

    vkCmdDraw(CMDBuffer,6,1,0,0);
    vkCmdEndRendering(CMDBuffer);

    swapchain->TransitionSwapchain(
      CMDBuffer,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_2_NONE,VK_ACCESS_2_NONE,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);



    auto result=vkEndCommandBuffer(CMDBuffer);
    if(result!=VK_SUCCESS)
      throw std::runtime_error("Unable to end command buffer");

    queue->Submit({CMDBuffer});

    queue->Wait();
    swapchain->PresentImage(queue);

    std::vector<uint32_t> queryResult(4,0);

    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}