#pragma once
#include<string>
#include<stdexcept>
#include<vulkan/vulkan.h>

struct Exception{
  VkResult Status;
  std::string Message;


};

