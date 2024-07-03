#include "VulkanShaderModule.h"
#include "dkpch.h"

#include "VulkanDevice.h"

#include <filesystem>

namespace Deako {

    VkShaderModule ShaderModule::Create(const std::string& filename)
    {
        auto shaderCode = ReadShaderFile(filename);

        VkDevice device = VulkanDevice::GetLogical();

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        // Size of the bytecode is in bytes, but bytecode pointer is a uint32_t pointer rather than a char pointer, thus need to reinterpret cast shader data
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        VkShaderModule shaderModule;
        VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
        DK_CORE_ASSERT(!result, "Failed to create shader module!");

        return shaderModule;
    }

    void ShaderModule::CleanUp(VkShaderModule shaderModule)
    {
        VkDevice device = VulkanDevice::GetLogical();
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }

    std::vector<char> ShaderModule::ReadShaderFile(const std::string& filename)
    {
        // Start reading at the end of the file (ate) to determine file size and read as binary file (binary)
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        DK_CORE_ASSERT(file.is_open(), "Failed to open shader file!");

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0); // go back to the beginning of the file
        file.read(buffer.data(), fileSize); // fill buffer with bytes

        file.close();

        return buffer;
    }

}
