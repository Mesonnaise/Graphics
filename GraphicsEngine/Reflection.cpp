#include<stdexcept>
#include<numeric>
#include<iostream>
#include<format>
#include<vulkan/vulkan.h>
#include "Reflection.h"

namespace Engine{
  static VkShaderStageFlagBits MapStage(spv::ExecutionModel em){
    switch(em){
      case spv::ExecutionModelVertex: return VK_SHADER_STAGE_VERTEX_BIT;
      case spv::ExecutionModelTessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
      case spv::ExecutionModelTessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
      case spv::ExecutionModelGeometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
      case spv::ExecutionModelFragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
      case spv::ExecutionModelGLCompute: return VK_SHADER_STAGE_COMPUTE_BIT;
      case spv::ExecutionModelRayGenerationKHR: return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
      case spv::ExecutionModelIntersectionKHR: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
      case spv::ExecutionModelAnyHitKHR: return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
      case spv::ExecutionModelClosestHitKHR: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
      case spv::ExecutionModelMissKHR: return VK_SHADER_STAGE_MISS_BIT_KHR;
      case spv::ExecutionModelCallableKHR: return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
      case spv::ExecutionModelTaskEXT: return VK_SHADER_STAGE_TASK_BIT_EXT;
      case spv::ExecutionModelMeshEXT: return VK_SHADER_STAGE_MESH_BIT_EXT;
      default:
        throw std::runtime_error(std::format("Unsupported shader stage"));
    }
  }

  Reflection::Reflection(std::vector<uint32_t> ByteCode):mHandle(ByteCode){
    auto eps=mHandle.get_entry_points_and_stages();
    if(eps.size()!=1)
      throw std::runtime_error("No entry point in shader");

    mEntryName=eps[0].name;
    mStage=MapStage(eps[0].execution_model);

    auto resources=mHandle.get_shader_resources();
    ProcessResourceBuffers(resources.uniform_buffers,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    ProcessResourceBuffers(resources.storage_buffers,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    processResourceImages(resources.sampled_images,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    processResourceImages(resources.separate_samplers,VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    processResourceImages(resources.storage_images,VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    for(auto push:resources.push_constant_buffers){
      mHasPush=true;
      
      auto baseType=mHandle.get_type(push.base_type_id);
      auto size=mHandle.get_declared_struct_size(baseType);
      mPush=std::make_tuple(push.name,size);
      break;
    }
  }

  void Reflection::ProcessResourceBuffers(spirv_cross::SmallVector<spirv_cross::Resource> &Resource,VkDescriptorType DescriptorType){
    for(auto r:Resource){
      uint32_t sIndex=mHandle.get_decoration(r.id,spv::DecorationDescriptorSet);
      uint32_t bIndex=mHandle.get_decoration(r.id,spv::DecorationBinding);
      auto &baseType=mHandle.get_type(r.base_type_id);
      auto &type=mHandle.get_type(r.type_id);

      //Need away to detect dynamic buffers and unbound sizes for last member of a struct


      /*for(auto memberId:baseType.member_types){
        auto memberType=mHandle.get_type(memberId);
        auto arraySize=memberType.array.size();
        auto value=memberType.array[0];
      }*/

      DescriptorLayoutBinding bs;
      bs.Binding.binding=bIndex;
      bs.Binding.descriptorType=DescriptorType;
      bs.Binding.stageFlags=mStage;
      bs.Binding.pImmutableSamplers=nullptr;
      bs.SetIndex=sIndex;
      bs.Name=r.name;
      auto temp=mHandle.get_declared_struct_size(baseType);

      if(DescriptorType==VK_DESCRIPTOR_TYPE_STORAGE_BUFFER||DescriptorType==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER){
        auto dimSize=type.array.size();
        bs.Size=mHandle.get_declared_struct_size(baseType);
        bs.Binding.descriptorCount=std::accumulate(
          type.array.begin(),type.array.end(),
          1,std::multiplies<uint32_t>());
      } else
        bs.Binding.descriptorCount=std::accumulate(
          type.array.begin(),type.array.end(),
          1,std::multiplies<uint32_t>());

      mBindings.push_back(bs);
    }
  }


  void Reflection::processResourceImages(spirv_cross::SmallVector<spirv_cross::Resource> &Resource,VkDescriptorType DescriptorType){
    for(auto r:Resource){
      uint32_t sIndex=mHandle.get_decoration(r.id,spv::DecorationDescriptorSet);
      uint32_t bIndex=mHandle.get_decoration(r.id,spv::DecorationBinding);
      auto &type=mHandle.get_type(r.type_id);

      DescriptorLayoutBinding bs;
      bs.Binding.binding=bIndex;
      bs.Binding.descriptorType=DescriptorType;
      bs.Binding.stageFlags=mStage;
      bs.Binding.pImmutableSamplers=nullptr;

      bs.Binding.descriptorCount=std::accumulate(
        type.array.begin(),type.array.end(),
        1,std::multiplies<uint32_t>());

      bs.SetIndex=sIndex;
      bs.Name=r.name;
      bs.Size=0;
      mBindings.push_back(bs);
    }
  }

  std::vector<DescriptorLayoutBinding> Reflection::DescriptorBindings()const{
    return mBindings;
  }

  std::string Reflection::EntryName()const{
    return mEntryName;
  }
  VkShaderStageFlagBits Reflection::Stage()const{
    return mStage;
  }
}