#include "VulkanBuffer.h"
#include "dkpch.h"

#include "VulkanDevice.h"
#include "VulkanCommand.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace Deako {

    VkDevice VulkanBufferPool::s_Device{ VK_NULL_HANDLE };
    Ref<VertexBuffer> VulkanBufferPool::s_VertexBuffer;
    Ref<IndexBuffer> VulkanBufferPool::s_IndexBuffer;
    VkDescriptorSetLayout VulkanBufferPool::s_DescriptorSetLayout{ VK_NULL_HANDLE };
    VkDescriptorPool VulkanBufferPool::s_DescriptorPool;
    std::array<VkDescriptorSet, VulkanBase::MAX_FRAMES_IN_FLIGHT> VulkanBufferPool::s_DescriptorSets;
    std::array<Ref<Buffer>, VulkanBase::MAX_FRAMES_IN_FLIGHT> VulkanBufferPool::s_UniformBuffers;

    VkVertexInputBindingDescription Vertex::GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        // specifies the index of the binding in the array of bindings
        bindingDescription.binding = 0;
        // specifies the number of bytes from one entry to the next
        bindingDescription.stride = sizeof(Vertex);
        // specifies whether to move to next data entry after each vertex or after each instance
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        // which binding the per-vertex data comes
        attributeDescriptions[0].binding = 0;
        // references the location directive of the input in the vertex shade
        attributeDescriptions[0].location = 0;
        // following shader types and formats are commonly used together:
        // • float: VK_FORMAT_R32_SFLOAT
        // • vec2: VK_FORMAT_R32G32_SFLOAT
        // • vec3: VK_FORMAT_R32G32B32_SFLOAT
        // • vec4: VK_FORMAT_R32G32B32A32_SFLOAT
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    Buffer::Buffer(VkDevice device)
        : m_Device(device)
    {
    }

    VertexBuffer::VertexBuffer(VkDevice device, const std::vector<Vertex>& vertices)
        : Buffer(device), m_Vertices(vertices)
    {
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

        Buffer stagingBuffer{ device };
        stagingBuffer.SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyTo(m_Vertices.data(), (size_t)bufferSize);
        stagingBuffer.Unmap();

        this->SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CopyStaging(stagingBuffer.GetBuffer(), this->GetBuffer(), bufferSize);

        vkDestroyBuffer(device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(device, stagingBuffer.GetMemory(), nullptr);
    }

    IndexBuffer::IndexBuffer(VkDevice device, const std::vector<uint16_t>& indices)
        : Buffer(device), m_Indices(indices)
    {
        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

        Buffer stagingBuffer{ device };
        stagingBuffer.SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyTo(m_Indices.data(), (size_t)bufferSize);
        stagingBuffer.Unmap();

        this->SetInfo(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CopyStaging(stagingBuffer.GetBuffer(), this->GetBuffer(), bufferSize);

        vkDestroyBuffer(device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(device, stagingBuffer.GetMemory(), nullptr);
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

        VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Buffer);
        DK_CORE_ASSERT(!result, "Failed to create vertex buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, m_Buffer, &memRequirements);

        // allocate the memory
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory);
        DK_CORE_ASSERT(!result, "Failed to allocate vertex buffer memory!");

        this->Bind();
    }

    void Buffer::CopyStaging(VkBuffer stagingBuffer, VkBuffer receivingBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = VulkanCommandPool::BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, receivingBuffer, 1, &copyRegion);

        VulkanCommandPool::EndSingleTimeCommands(commandBuffer);
    }

    void Buffer::Bind(VkDeviceSize offset)
    {
        VkResult result = vkBindBufferMemory(m_Device, m_Buffer, m_Memory, offset);
        DK_CORE_ASSERT(!result, "Failed to bind vertex buffer memory!");
    }

    void Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        VkResult result = vkMapMemory(m_Device, m_Memory, offset, size, 0, &m_Mapped);
        DK_CORE_ASSERT(!result, "Failed to bind vertex buffer memory!");
    }

    void Buffer::Unmap()
    {
        if (m_Mapped)
        {
            vkUnmapMemory(m_Device, m_Memory);
            m_Mapped = nullptr;
        }
    }

    void Buffer::CopyTo(const void* data, VkDeviceSize size)
    {
        DK_CORE_ASSERT(m_Mapped, "Buffer is not mapped!");
        memcpy(m_Mapped, data, size);
    }

    uint32_t Buffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDevice physicalDevice = VulkanDevice::GetPhysical();

        // query info about the available types of memory from physical device
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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

        DK_CORE_ASSERT(false, "Failed to find suitable memory type!");
        return 0;
    }

    void VulkanBufferPool::CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        for (size_t i = 0; i < VulkanBase::MAX_FRAMES_IN_FLIGHT; i++)
        {
            s_UniformBuffers[i] = CreateRef<Buffer>(s_Device);
            s_UniformBuffers[i]->SetInfo(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            vkMapMemory(s_Device, s_UniformBuffers[i]->GetMemory(), 0, bufferSize, 0, &s_UniformBuffers[i]->Mapped());
        }

        CreateDescriptorPool();
        CreateDescriptorSets();
    }

    void VulkanBufferPool::CreateVertexBuffers()
    {
        const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        s_VertexBuffer = CreateRef<VertexBuffer>(s_Device, vertices);
    }

    void VulkanBufferPool::CreateIndexBuffer()
    {
        const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };
        s_IndexBuffer = CreateRef<IndexBuffer>(s_Device, indices);
    }

    void VulkanBufferPool::CreateDescriptorSetLayout()
    {
        s_Device = VulkanDevice::GetLogical();

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult result = vkCreateDescriptorSetLayout(s_Device, &layoutInfo, nullptr, &s_DescriptorSetLayout);
        DK_CORE_ASSERT(!result, "Failed to create descriptor set layout!");
    }

    void VulkanBufferPool::CreateDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(VulkanBase::MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(VulkanBase::MAX_FRAMES_IN_FLIGHT) * 2; // imgui requires * 2 for extra texture

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(VulkanBase::MAX_FRAMES_IN_FLIGHT) * 2;  // imgui requires * 2 for extra texture

        VkResult result = vkCreateDescriptorPool(s_Device, &poolInfo, nullptr, &s_DescriptorPool);
        DK_CORE_ASSERT(!result, "Failed to create descriptor pool!");
    }

    void VulkanBufferPool::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(VulkanBase::MAX_FRAMES_IN_FLIGHT, s_DescriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = s_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanBase::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        VkResult result = vkAllocateDescriptorSets(s_Device, &allocInfo, s_DescriptorSets.data());
        DK_CORE_ASSERT(!result, "Failed to allocate descriptor sets!");

        for (size_t i = 0; i < VulkanBase::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = s_UniformBuffers[i]->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = VulkanTexturePool::GetTexture()->GetImageView();
            imageInfo.sampler = VulkanTexturePool::GetTextureSampler()->GetSampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = s_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = s_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(s_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void VulkanBufferPool::UpdateUniformBuffer(uint32_t currentImage)
    {
        VkExtent2D swapChainExtent = VulkanSwapChain::GetExtent();
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.projection = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        ubo.projection[1][1] *= -1;

        s_UniformBuffers[currentImage]->CopyTo(&ubo, sizeof(ubo));
    }

    void VulkanBufferPool::CleanUp()
    {
        vkDestroyBuffer(s_Device, s_VertexBuffer->GetBuffer(), nullptr);
        vkFreeMemory(s_Device, s_VertexBuffer->GetMemory(), nullptr);

        vkDestroyBuffer(s_Device, s_IndexBuffer->GetBuffer(), nullptr);
        vkFreeMemory(s_Device, s_IndexBuffer->GetMemory(), nullptr);

        for (size_t i = 0; i < VulkanBase::MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(s_Device, s_UniformBuffers[i]->GetBuffer(), nullptr);
            vkFreeMemory(s_Device, s_UniformBuffers[i]->GetMemory(), nullptr);
        }

        vkDestroyDescriptorPool(s_Device, s_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(s_Device, s_DescriptorSetLayout, nullptr);
    }

}
