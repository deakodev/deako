#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class ShaderModule
    {
    public:
        static VkShaderModule Create(const std::string& filename);
        static void CleanUp(VkShaderModule shaderModule);

        static std::vector<char> ReadShaderFile(const std::string& filename);

    private:
        static Ref<VulkanResources> s_VR;
    };

}
