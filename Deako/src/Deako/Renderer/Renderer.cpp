#include "Renderer.h"
#include "dkpch.h"

#include "System/Vulkan/VulkanBase.h"

namespace Deako {

    void Renderer::Init(const char* appName)
    {
        VulkanBase::Init(appName);
    }

    void Renderer::Shutdown()
    {
        VulkanBase::Idle();
        VulkanBase::Shutdown();
    }

}
