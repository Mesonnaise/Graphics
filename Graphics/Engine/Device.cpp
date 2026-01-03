#include<stdexcept>
#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>
#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"

Device::Queue::Queue(std::shared_ptr<Device> device,uint32_t QueueFamily,uint32_t QueueIndex):mDevice(device),mQueueIndex(QueueIndex){
  vkGetDeviceQueue(mDevice->Handle(),QueueFamily,0,&mHandle);
}


void Device::Queue::Submit(std::vector<VkCommandBuffer> CommandBuffers){

  std::vector<VkSubmitInfo> infos;

  VkSubmitInfo info={
    .sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext=nullptr,
    .waitSemaphoreCount=0,
    .pWaitSemaphores=nullptr,
    .pWaitDstStageMask=nullptr,
    .commandBufferCount=(uint32_t)CommandBuffers.size(),
    .pCommandBuffers=CommandBuffers.data(),
    .signalSemaphoreCount=0,
    .pSignalSemaphores=nullptr
  };
  infos.push_back(info);

  vkQueueSubmit(mHandle,(uint32_t)infos.size(),infos.data(),nullptr);
}

void Device::Queue::Wait(){
  vkQueueWaitIdle(mHandle);
}

Device::Device(InstancePtr ins,Instance::Physical phy,std::optional<SurfacePtr> surface):mInstance(ins),mPhysical(phy){
  std::vector<VkDeviceQueueCreateInfo> queueInfo;
  auto queueProps=phy.QueueFamilyProperties();
 
  std::vector<const char *> DeviceExtension;
  DeviceExtension.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  DeviceExtension.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
  VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputDynamicStateFeature={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
    .pNext=nullptr,
    .vertexInputDynamicState=VK_TRUE
  };

  
  DeviceExtension.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
  VkPhysicalDeviceMeshShaderFeaturesEXT meshShader={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
    .pNext=&vertexInputDynamicStateFeature,
    .taskShader=VK_FALSE,
    .meshShader=VK_FALSE,
    .multiviewMeshShader=VK_FALSE,
    .primitiveFragmentShadingRateMeshShader=VK_FALSE,
    .meshShaderQueries=VK_FALSE
  };



  DeviceExtension.push_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
  VkPhysicalDeviceDescriptorBufferFeaturesEXT DescriptorBufferFeatures={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
    .pNext=&meshShader,
    .descriptorBuffer=VK_TRUE,
    .descriptorBufferCaptureReplay=VK_FALSE,
    .descriptorBufferImageLayoutIgnored=VK_FALSE,
    .descriptorBufferPushDescriptors=VK_FALSE
  };

  DeviceExtension.push_back(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
  VkPhysicalDeviceShaderObjectFeaturesEXT ShaderObjectFeatures={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
    .pNext=&DescriptorBufferFeatures,
    .shaderObject=VK_TRUE
  };

  VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
    .pNext=&ShaderObjectFeatures,
    .synchronization2=VK_TRUE
  };

  VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
    .pNext=&sync2Features,
    .bufferDeviceAddress=VK_TRUE,
    .bufferDeviceAddressCaptureReplay=VK_FALSE,
    .bufferDeviceAddressMultiDevice=VK_FALSE
  };

  VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    .pNext=&bufferDeviceAddressFeatures,
    .dynamicRendering=VK_TRUE
  };

  auto FindQueue=[&queueProps](uint32_t queueFlags,uint32_t startIndex)->int32_t{
    for(uint32_t i=startIndex;i<queueProps.size();i++){
      if((queueProps[i].queueFlags&queueFlags)==queueFlags){
        return i;
      }
    }
    return -1;
  };
  const float QueuePrimiary=1.0f;

  auto AddQueue=[&queueInfo,&QueuePrimiary](uint32_t queueFamilyIndex){
    VkDeviceQueueCreateInfo info={
      .sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext=nullptr,
      .flags=0,
      .queueFamilyIndex=queueFamilyIndex,
      .queueCount=1,
      .pQueuePriorities=&QueuePrimiary,

    };

    queueInfo.push_back(info);
  };

  int32_t QueueIndex=0;
  if(surface.has_value()){
    while(QueueIndex<queueProps.size()){
      QueueIndex=FindQueue(VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT|VK_QUEUE_TRANSFER_BIT,0);

      if(phy.IsSurfaceSupported(QueueIndex,surface.value())){
        AddQueue(QueueIndex);
        break;
      }
    }
    if(QueueIndex==-1||QueueIndex>queueProps.size())
      throw std::runtime_error("Unable to find supported queue");

  } else{
    QueueIndex=FindQueue(VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT|VK_QUEUE_TRANSFER_BIT,0);
  }
  mPrimaryQueueFamily=QueueIndex;

  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(mPhysical.Handle(),&features);
  features.vertexPipelineStoresAndAtomics=VK_TRUE;
  features.fragmentStoresAndAtomics=VK_TRUE;
  features.robustBufferAccess=VK_TRUE;
  
  VkDeviceCreateInfo info={
    .sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext=&dynamicRenderingFeatures,
    .flags=0,
    .queueCreateInfoCount=(uint32_t)queueInfo.size(),
    .pQueueCreateInfos=queueInfo.data(),
    .enabledLayerCount=0,
    .ppEnabledLayerNames=nullptr,
    .enabledExtensionCount=(uint32_t)DeviceExtension.size(),
    .ppEnabledExtensionNames=DeviceExtension.data(),
    .pEnabledFeatures=&features,
  };


  auto status=vkCreateDevice(phy.Handle(),&info,nullptr,&mHandle);
  if(status!=VK_SUCCESS)
    throw std::runtime_error("Unable to create device");
}

Device::~Device(){
  vkDestroyDevice(mHandle,nullptr);
}

VkPhysicalDeviceDescriptorBufferPropertiesEXT Device::GetDescriptorProperties()const{
  VkPhysicalDeviceDescriptorBufferPropertiesEXT properties={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
    .pNext=nullptr
  };

  VkPhysicalDeviceProperties2 physicalProperties={
    .sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    .pNext=&properties
  };

  vkGetPhysicalDeviceProperties2(mPhysical.Handle(),&physicalProperties);

  return properties;
}

SwapchainPtr Device::CreateSwapchain(SurfacePtr surface){
  return Swapchain::Create(shared_from_this(),surface);
}

void Device::Debug(VkObjectType type,uint64_t handle,std::string name){
  auto pfnSetDebugUtilsObjectNameEXT=reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
    vkGetDeviceProcAddr(mHandle,"vkSetDebugUtilsObjectNameEXT"));

  VkDebugUtilsObjectNameInfoEXT debugUtilsNameInfo={
  .sType=VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
  .pNext=nullptr,
  .objectType=type,
  .objectHandle=handle,
  .pObjectName=name.c_str()
  };

  pfnSetDebugUtilsObjectNameEXT(mHandle,&debugUtilsNameInfo);
}