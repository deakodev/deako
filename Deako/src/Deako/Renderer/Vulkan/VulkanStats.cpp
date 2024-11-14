#include "VulkanStats.h"
#include "dkpch.h"
#include "VulkanBase.h"

namespace Deako {

    static Ref<VulkanBaseResources> vb = VulkanBase::GetResources();

    namespace VulkanStats
    {

        void SetupTimestampQuery()
        {
            VkQueryPoolCreateInfo queryPoolInfo = {};
            queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolInfo.queryCount = 2;

            VkCR(vkCreateQueryPool(vb->device, &queryPoolInfo, nullptr, &vb->timestampQueryPool));
        }

        void RecordStartTime(VkCommandBuffer commandBuffer)
        {
            vkCmdResetQueryPool(commandBuffer, vb->timestampQueryPool, 0, 2);
            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vb->timestampQueryPool, 0);
        }

        void RecordEndTime(VkCommandBuffer commandBuffer)
        {
            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vb->timestampQueryPool, 1);
        }

        void QueryTimestamps()
        {
            uint64_t timestamps[2];
            vkGetQueryPoolResults(vb->device, vb->timestampQueryPool, 0, 2, sizeof(timestamps),
                timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

            uint64_t startTime = timestamps[0];
            uint64_t endTime = timestamps[1];

            vb->stats.frameTime = (endTime - startTime) / 1e6;  // Frame time in milliseconds
        }
    }
}
