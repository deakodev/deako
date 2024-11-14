#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    namespace VulkanStats
    {

        void SetupTimestampQuery();
        void RecordStartTime(VkCommandBuffer commandBuffer);
        void RecordEndTime(VkCommandBuffer commandBuffer);
        void QueryTimestamps();

    }
}
