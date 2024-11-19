#include "VulkanStats.h"
#include "dkpch.h"

#include "VulkanBase.h"

namespace Deako {
    namespace VulkanStats
    {

        void SetupTimestampQuery()
        {
            VulkanBaseResources& vb = VulkanBase::GetResources();

            VkQueryPoolCreateInfo queryPoolInfo = {};
            queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolInfo.queryCount = 2;

            VkCR(vkCreateQueryPool(vb.device, &queryPoolInfo, nullptr, &vb.timestampQueryPool));
        }

        void RecordStartTime(VkCommandBuffer commandBuffer)
        {
            VulkanBaseResources& vb = VulkanBase::GetResources();

            vkCmdResetQueryPool(commandBuffer, vb.timestampQueryPool, 0, 2);
            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vb.timestampQueryPool, 0);
        }

        void RecordEndTime(VkCommandBuffer commandBuffer)
        {
            VulkanBaseResources& vb = VulkanBase::GetResources();

            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vb.timestampQueryPool, 1);
        }

        DkU64 QueryTimestamps()
        {
            VulkanBaseResources& vb = VulkanBase::GetResources();

            DkU64 timestamps[2];
            vkGetQueryPoolResults(vb.device, vb.timestampQueryPool, 0, 2, sizeof(timestamps),
                timestamps, sizeof(DkU64), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

            DkU64 startTime = timestamps[0];
            DkU64 endTime = timestamps[1];

            DkU64 deltaTime = (endTime - startTime) / 1e6;  // in milliseconds;

            return deltaTime;
        }

    }
}
