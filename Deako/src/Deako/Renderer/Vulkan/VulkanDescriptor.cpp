#include "VulkanDescriptor.h"
#include "dkpch.h"

#include "VulkanBase.h"

namespace Deako {

    static Ref<VulkanBaseResources> vb = VulkanBase::GetResources();

    namespace VulkanDescriptor {

        void LayoutBuilder::AddBinding(uint32_t bindingIndex, VkDescriptorType type, VkShaderStageFlags flags)
        {
            VkDescriptorSetLayoutBinding binding{};

            binding.binding = bindingIndex;
            binding.descriptorCount = 1;
            binding.descriptorType = type;
            binding.stageFlags = flags;

            bindings.push_back(binding);
        }

        VkDescriptorSetLayout LayoutBuilder::Build(void* pNext, VkDescriptorSetLayoutCreateFlags flags)
        {
            VkDescriptorSetLayoutCreateInfo layoutInfo{};

            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pBindings = bindings.data();
            layoutInfo.bindingCount = (uint32_t)bindings.size();
            layoutInfo.pNext = pNext;

            VkDescriptorSetLayout descriptorSetLayout;
            VkCR(vkCreateDescriptorSetLayout(vb->device, &layoutInfo, nullptr, &descriptorSetLayout));

            return descriptorSetLayout;
        }

        void Writer::WriteBuffer(uint32_t bindingIndex, VkDescriptorSet descriptorSet, VkDescriptorType descriptorType, VkDescriptorBufferInfo bufferInfo)
        {
            VkWriteDescriptorSet write{};

            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstBinding = bindingIndex;
            write.dstSet = descriptorSet;
            write.descriptorType = descriptorType;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfo;

            writes.push_back(write);
        }


        void Writer::WriteImage(uint32_t bindingIndex, VkDescriptorSet descriptorSet, VkDescriptorType descriptorType, VkDescriptorImageInfo imageInfo)
        {
            VkWriteDescriptorSet write{};

            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstBinding = bindingIndex;
            write.dstSet = descriptorSet;
            write.descriptorType = descriptorType;
            write.descriptorCount = 1;
            write.pImageInfo = &imageInfo;

            writes.push_back(write);
        }

        void Writer::UpdateSets()
        {
            vkUpdateDescriptorSets(vb->device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
        }

        AllocatorGrowable::AllocatorGrowable(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
        {
            m_Ratios.reserve(poolRatios.size());
            for (auto ratio : poolRatios) m_Ratios.emplace_back(ratio);

            VkDescriptorPool descriptorPool = CreatePool(maxSets, poolRatios);

            m_SetsPerPool = maxSets * 1.5; // grow the next allocation

            m_ReadyPools.emplace_back(descriptorPool);
        }

        VkDescriptorSet AllocatorGrowable::Allocate(VkDescriptorSetLayout descriptorLayout, void* pNext)
        {
            VkDescriptorSet descriptorSet{};

            VkDescriptorPool poolToUse = GetPool(); //get or create pool to allocate from

            VkDescriptorSetAllocateInfo allocInfo{};

            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = poolToUse;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &descriptorLayout;
            allocInfo.pNext = pNext;

            VkResult result = vkAllocateDescriptorSets(vb->device, &allocInfo, &descriptorSet);

            if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
            {   // allocation failed. Try again

                m_FullPools.emplace_back(poolToUse);

                poolToUse = GetPool();
                allocInfo.descriptorPool = poolToUse;

                VkCR(vkAllocateDescriptorSets(vb->device, &allocInfo, &descriptorSet));
            }

            m_ReadyPools.emplace_back(poolToUse);

            return descriptorSet;
        }

        VkDescriptorPool AllocatorGrowable::CreatePool(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
        {
            VkDescriptorPool descriptorPool{};

            std::vector<VkDescriptorPoolSize> poolSizes;
            poolSizes.reserve(poolRatios.size());
            for (auto& ratio : poolRatios)
            {
                VkDescriptorType descriptorType = ratio.type;
                uint32_t descriptorCount = ratio.ratio * maxSets;
                poolSizes.emplace_back(descriptorType, descriptorCount);
            }

            VkDescriptorPoolCreateInfo poolInfo{};

            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = 0;
            poolInfo.maxSets = maxSets;
            poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
            poolInfo.pPoolSizes = poolSizes.data();

            vkCreateDescriptorPool(vb->device, &poolInfo, nullptr, &descriptorPool);

            return descriptorPool;
        }

        VkDescriptorPool AllocatorGrowable::GetPool()
        {
            VkDescriptorPool descriptorPool{};

            if (m_ReadyPools.size() != 0)
            {
                descriptorPool = m_ReadyPools.back();
                m_ReadyPools.pop_back();
            }
            else
            {   // need to create a new pool
                descriptorPool = CreatePool(m_SetsPerPool, m_Ratios);

                m_SetsPerPool = m_SetsPerPool * 1.5;

                if (m_SetsPerPool > 4092) m_SetsPerPool = 4092;
            }

            return descriptorPool;
        }

        void AllocatorGrowable::DestroyPools()
        {
            for (auto pool : m_ReadyPools)
                vkDestroyDescriptorPool(vb->device, pool, nullptr);
            for (auto pool : m_FullPools)
                vkDestroyDescriptorPool(vb->device, pool, nullptr);

            m_ReadyPools.clear();
            m_FullPools.clear();
        }

    }
}
