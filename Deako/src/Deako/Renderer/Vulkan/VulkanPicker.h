#pragma once

#include "VulkanResource.h"
#include "VulkanDescriptor.h"

#include <vulkan/vulkan.h>

namespace Deako {

    struct VulkanPickerResources
    {
        std::vector<UniformBuffer>                 uniforms;
        size_t                                     dynamicUniformAlignment{ 0 };

        struct
        {   // per-object uniform data
            DkVec4* colorID{ nullptr };
        } uniformDynamicData;

        AllocatedImage                             colorTarget;
        AllocatedImage                             depthTarget;
        AllocatedBuffer                            stagingBuffer;

        VulkanDescriptor::AllocatorGrowable        staticDescriptorAllocator; // vs per-frame
        VkDescriptorSetLayout                      descriptorLayout;
        VkDescriptorSet                            descriptorSet;
        VkPipelineLayout                           pipelineLayout;
        VkPipeline                                 pipeline;
    };

    class VulkanPicker
    {
    public:
        static void Init();
        static void CleanUp();

        static void OnUpdate();

        static void Draw(VkCommandBuffer commandBuffer, DkU32 imageIndex);

        static VulkanPickerResources& GetResources() { return s_Resources; }
        static const DkVec4& GetPixelColor() { return *static_cast<DkVec4*>(s_Resources.stagingBuffer.mapped); }

    private:
        static void SetUpTargets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipeline();

    private:
        inline static VulkanPickerResources s_Resources;
    };

}
