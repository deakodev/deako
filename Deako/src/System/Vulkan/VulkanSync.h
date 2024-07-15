#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class Sync
    {
    public:
        static void CreateObjects();
        static void CleanUp();

    private:
        static Ref<VulkanResources> s_VR;
        static Ref<VulkanSettings> s_VS;
    };

}
