#pragma once

#include <vulkan/vulkan.h>

#include <span>

namespace Deako {
    namespace VulkanDescriptor {

        struct LayoutBuilder
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            void AddBinding(uint32_t bindingIndex, VkDescriptorType type, VkShaderStageFlags flags = 0);

            VkDescriptorSetLayout Build(void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
        };

        struct Writer
        {
            std::vector<VkWriteDescriptorSet> writes;

            void WriteBuffer(uint32_t bindingIndex, VkDescriptorSet descriptorSet, VkDescriptorType descriptorType, VkDescriptorBufferInfo bufferInfo);
            void WriteImage(uint32_t bindingIndex, VkDescriptorSet descriptorSet, VkDescriptorType descriptorType, VkDescriptorImageInfo imageInfo);

            void UpdateSets();
        };

        struct PoolSizeRatio
        {
            VkDescriptorType type;
            float ratio;
        };

        class AllocatorGrowable
        {
        public:
            AllocatorGrowable() = default;
            AllocatorGrowable(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);

            VkDescriptorSet Allocate(VkDescriptorSetLayout descriptorLayout, void* pNext = nullptr);
            void DestroyPools();

        private:
            VkDescriptorPool CreatePool(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
            VkDescriptorPool GetPool();

        private:
            std::vector<PoolSizeRatio> m_Ratios;
            std::vector<VkDescriptorPool> m_FullPools;
            std::vector<VkDescriptorPool> m_ReadyPools;
            uint32_t m_SetsPerPool;
        };

    }
}
