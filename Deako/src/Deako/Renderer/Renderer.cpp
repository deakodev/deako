#include "Renderer.h"
#include "dkpch.h"

#include "System/Vulkan/VulkanBase.h"

namespace Deako {

    void Renderer::Init()
    {
        VulkanBase::Init();
    }

    void Renderer::Shutdown()
    {
        VulkanBase::Idle();
        VulkanBase::Shutdown();
    }

}
