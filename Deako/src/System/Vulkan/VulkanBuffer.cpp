#include "VulkanBuffer.h"
#include "dkpch.h"

#include "VulkanCommand.h"
#include "VulkanTexture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace Deako {

    Ref<VertexBuffer> BufferPool::s_VertexBuffer;
    Ref<IndexBuffer> BufferPool::s_IndexBuffer;
    std::array<VkDescriptorSet, 2> BufferPool::s_DescriptorSets;
    std::array<Ref<Buffer>, 2> BufferPool::s_UniformBuffers;
    Ref<VulkanResources> BufferPool::s_VR = VulkanBase::GetResources();
    Ref<VulkanResources> Buffer::s_VR = VulkanBase::GetResources();
    Ref<VulkanSettings> BufferPool::s_VS = VulkanBase::GetSettings();
    Ref<VulkanSettings> Buffer::s_VS = VulkanBase::GetSettings();


    std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescription()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};

        bindingDescriptions.emplace_back(
            VulkanInitializers::VertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
        );

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.emplace_back(
            VulkanInitializers::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position))
        );
        attributeDescriptions.emplace_back(
            VulkanInitializers::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
        );
        attributeDescriptions.emplace_back(
            VulkanInitializers::VertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord))
        );

        return attributeDescriptions;
    }

    VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices)
        : m_Vertices(vertices)
    {
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

        Buffer stagingBuffer{};
        stagingBuffer.SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyTo(m_Vertices.data(), (size_t)bufferSize);
        stagingBuffer.Unmap();

        this->SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CopyStaging(stagingBuffer.GetBuffer(), this->GetBuffer(), bufferSize);

        vkDestroyBuffer(s_VR->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, stagingBuffer.GetMemory(), nullptr);
    }

    IndexBuffer::IndexBuffer(const std::vector<uint16_t>& indices)
        : m_Indices(indices)
    {
        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

        Buffer stagingBuffer{};
        stagingBuffer.SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyTo(m_Indices.data(), (size_t)bufferSize);
        stagingBuffer.Unmap();

        this->SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CopyStaging(stagingBuffer.GetBuffer(), this->GetBuffer(), bufferSize);

        vkDestroyBuffer(s_VR->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, stagingBuffer.GetMemory(), nullptr);
    }

    void Buffer::SetInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        // indicates for which purposes the data in the buffer is going to be used
        bufferInfo.usage = usage;
        // buffers can be owned by a specific queue family or be shared
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(s_VR->device, &bufferInfo, nullptr, &m_Buffer);
        DK_CORE_ASSERT(!result);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(s_VR->device, m_Buffer, &memRequirements);

        // allocate the memory
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(s_VR->device, &allocInfo, nullptr, &m_Memory);
        DK_CORE_ASSERT(!result);

        this->Bind();
    }

    void Buffer::CopyStaging(VkBuffer stagingBuffer, VkBuffer receivingBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = CommandPool::BeginSingleTimeCommands(s_VR->commandPool);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, receivingBuffer, 1, &copyRegion);

        CommandPool::EndSingleTimeCommands(s_VR->commandPool, commandBuffer);
    }

    void Buffer::Bind(VkDeviceSize offset)
    {
        VkResult result = vkBindBufferMemory(s_VR->device, m_Buffer, m_Memory, offset);
        DK_CORE_ASSERT(!result);
    }

    void Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        VkResult result = vkMapMemory(s_VR->device, m_Memory, offset, size, 0, &m_Mapped);
        DK_CORE_ASSERT(!result);
    }

    void Buffer::Unmap()
    {
        if (m_Mapped)
        {
            vkUnmapMemory(s_VR->device, m_Memory);
            m_Mapped = nullptr;
        }
    }

    void Buffer::CopyTo(const void* data, VkDeviceSize size)
    {
        DK_CORE_ASSERT(m_Mapped);
        memcpy(m_Mapped, data, size);
    }

    uint32_t Buffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        // query info about the available types of memory from physical device
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(s_VR->physicalDevice, &memProperties);

        // typeFilter - will be used to specify the bit field of memory types that are suitable. We can find the index of a suitable memory type by iterating over them and checking if corresponding bit is set to 1
        // properties - define special features of the memory, like being able to map it. May have more than one desirable property, so we check if the result of bitwise AND is not just non-zero, but equal to the desired properties bit field. If a memory type suitable for the buffer that also has all of the properties we need, then we return the index
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if (typeFilter & (1 << i)
                && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        DK_CORE_ASSERT(false);
        return 0;
    }

    void BufferPool::CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        for (size_t i = 0; i < s_VS->imageCount; i++)
        {
            s_UniformBuffers[i] = CreateRef<Buffer>();
            s_UniformBuffers[i]->SetInfo(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            vkMapMemory(s_VR->device, s_UniformBuffers[i]->GetMemory(), 0, bufferSize, 0, &s_UniformBuffers[i]->Mapped());
        }
    }

    void BufferPool::CreateVertexBuffers()
    {
        const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
        };
        s_VertexBuffer = CreateRef<VertexBuffer>(vertices);
    }

    void BufferPool::CreateIndexBuffer()
    {
        const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };
        s_IndexBuffer = CreateRef<IndexBuffer>(indices);
    }

    void BufferPool::CreateDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding =
            VulkanInitializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
        VkDescriptorSetLayoutBinding samplerLayoutBinding =
            VulkanInitializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(bindings);

        VkResult result = vkCreateDescriptorSetLayout(s_VR->device, &layoutInfo, nullptr, &s_VR->descriptorSetLayout);
        DK_CORE_ASSERT(!result);
    }

    void BufferPool::CreateDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };

        VkDescriptorPoolCreateInfo poolInfo = VulkanInitializers::DescriptorPoolCreateInfo(poolSizes, 1000 * poolSizes.size());
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        VkResult result = vkCreateDescriptorPool(s_VR->device, &poolInfo, nullptr, &s_VR->descriptorPool);
        DK_CORE_ASSERT(!result);
    }

    void BufferPool::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(s_VS->imageCount, s_VR->descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo =
            VulkanInitializers::DescriptorSetAllocateInfo(s_VR->descriptorPool, layouts.data(), static_cast<uint32_t>(s_VS->imageCount));

        VkResult result = vkAllocateDescriptorSets(s_VR->device, &allocInfo, s_DescriptorSets.data());
        DK_CORE_ASSERT(!result);

        for (size_t i = 0; i < s_VS->imageCount; i++)
        {
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            // Uniform Buffer
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = s_UniformBuffers[i]->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = s_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            // Viewport Image
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = TexturePool::GetViewportTexture()->GetImageView();
            imageInfo.sampler = TexturePool::GetTextureSampler();

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = s_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(s_VR->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void BufferPool::UpdateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.projection = glm::perspective(glm::radians(45.0f), s_VR->imageExtent.width / (float)s_VR->imageExtent.height, 0.1f, 10.0f);
        ubo.projection[1][1] *= -1;

        s_UniformBuffers[currentImage]->CopyTo(&ubo, sizeof(ubo));
    }

    void BufferPool::CleanUp()
    {
        vkDestroyBuffer(s_VR->device, s_VertexBuffer->GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, s_VertexBuffer->GetMemory(), nullptr);

        vkDestroyBuffer(s_VR->device, s_IndexBuffer->GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, s_IndexBuffer->GetMemory(), nullptr);

        for (size_t i = 0; i < s_VS->imageCount; i++)
        {
            vkDestroyBuffer(s_VR->device, s_UniformBuffers[i]->GetBuffer(), nullptr);
            vkFreeMemory(s_VR->device, s_UniformBuffers[i]->GetMemory(), nullptr);
        }

        vkDestroyDescriptorPool(s_VR->device, s_VR->descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(s_VR->device, s_VR->descriptorSetLayout, nullptr);
    }

}
