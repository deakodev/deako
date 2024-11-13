#pragma once

#include <vulkan/vulkan.h>

#include <span>

namespace Deako {
    namespace VulkanDescriptor {

        struct LayoutBuilder
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            void AddBinding(DkU32 bindingIndex, VkDescriptorType type, VkShaderStageFlags flags = 0);

            VkDescriptorSetLayout Build(void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
        };

        struct Writer
        {
            std::vector<VkWriteDescriptorSet> writes;

            void WriteBuffer(DkU32 bindingIndex, VkDescriptorSet descriptorSet, VkDescriptorType descriptorType, VkDescriptorBufferInfo bufferInfo);
            void WriteImage(DkU32 bindingIndex, VkDescriptorSet descriptorSet, VkDescriptorType descriptorType, VkDescriptorImageInfo imageInfo);

            void UpdateSets();
        };

        struct PoolSizeRatio
        {
            VkDescriptorType type;
            DkF32 ratio;
        };

        class AllocatorGrowable
        {
        public:
            AllocatorGrowable() = default;
            AllocatorGrowable(DkU32 maxSets, std::span<PoolSizeRatio> poolRatios);

            VkDescriptorSet Allocate(VkDescriptorSetLayout descriptorLayout, void* pNext = nullptr);
            void DestroyPools();

        private:
            VkDescriptorPool CreatePool(DkU32 maxSets, std::span<PoolSizeRatio> poolRatios);
            VkDescriptorPool GetPool();

        private:
            std::vector<PoolSizeRatio> m_Ratios;
            std::vector<VkDescriptorPool> m_FullPools;
            std::vector<VkDescriptorPool> m_ReadyPools;
            DkU32 m_SetsPerPool;
        };

    }
}
