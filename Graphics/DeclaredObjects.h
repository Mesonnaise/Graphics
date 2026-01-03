#pragma once
#include<memory>


class Instance;
class Swapchain;
class Surface;
class Device;

using InstancePtr=std::shared_ptr<Instance>;
using SwapchainPtr=std::shared_ptr<Swapchain>;
using SurfacePtr=std::shared_ptr<Surface>;
using DevicePtr=std::shared_ptr<Device>;