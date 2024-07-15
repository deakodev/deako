#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class RenderPass
    {
    public:
        static void Create();
        static void CleanUp();

    private:
        static Ref<VulkanResources> s_VR;
    };

}
