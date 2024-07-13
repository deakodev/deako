#include "VulkanBuffer.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanCommand.h"
#include "VulkanTexture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace Deako {

    Ref<VertexBuffer> VulkanBufferPool::s_VertexBuffer;
    Ref<IndexBuffer> VulkanBufferPool::s_IndexBuffer;
    std::array<VkDescriptorSet, 2> VulkanBufferPool::s_DescriptorSets;
    std::array<Ref<Buffer>, 2> VulkanBufferPool::s_UniformBuffers;

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
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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

    VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices)
        : m_Vertices(vertices)
    {
        VulkanResources* vr = VulkanBase::GetResources();

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

        vkDestroyBuffer(vr->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(vr->device, stagingBuffer.GetMemory(), nullptr);
    }

    IndexBuffer::IndexBuffer(const std::vector<uint16_t>& indices)
        : m_Indices(indices)
    {
        VulkanResources* vr = VulkanBase::GetResources();

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

        vkDestroyBuffer(vr->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(vr->device, stagingBuffer.GetMemory(), nullptr);
    }

    void Buffer::SetInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        // indicates for which purposes the data in the buffer is going to be used
        bufferInfo.usage = usage;
        // buffers can be owned by a specific queue family or be shared
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(vr->device, &bufferInfo, nullptr, &m_Buffer);
        DK_CORE_ASSERT(!result);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vr->device, m_Buffer, &memRequirements);

        // allocate the memory
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(vr->device, &allocInfo, nullptr, &m_Memory);
        DK_CORE_ASSERT(!result);

        this->Bind();
    }

    void Buffer::CopyStaging(VkBuffer stagingBuffer, VkBuffer receivingBuffer, VkDeviceSize size)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkCommandBuffer commandBuffer = VulkanCommandPool::BeginSingleTimeCommands(vr->commandPool);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, receivingBuffer, 1, &copyRegion);

        VulkanCommandPool::EndSingleTimeCommands(vr->commandPool, commandBuffer);
    }

    void Buffer::Bind(VkDeviceSize offset)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkResult result = vkBindBufferMemory(vr->device, m_Buffer, m_Memory, offset);
        DK_CORE_ASSERT(!result);
    }

    void Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkResult result = vkMapMemory(vr->device, m_Memory, offset, size, 0, &m_Mapped);
        DK_CORE_ASSERT(!result);
    }

    void Buffer::Unmap()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        if (m_Mapped)
        {
            vkUnmapMemory(vr->device, m_Memory);
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
        VulkanResources* vr = VulkanBase::GetResources();

        // query info about the available types of memory from physical device
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(vr->physicalDevice, &memProperties);

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

    void VulkanBufferPool::CreateUniformBuffers()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        for (size_t i = 0; i < vr->imageCount; i++)
        {
            s_UniformBuffers[i] = CreateRef<Buffer>();
            s_UniformBuffers[i]->SetInfo(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            vkMapMemory(vr->device, s_UniformBuffers[i]->GetMemory(), 0, bufferSize, 0, &s_UniformBuffers[i]->Mapped());
        }

        CreateDescriptorPool();
        CreateDescriptorSets();
    }

    void VulkanBufferPool::CreateVertexBuffers()
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

    void VulkanBufferPool::CreateIndexBuffer()
    {
        const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };
        s_IndexBuffer = CreateRef<IndexBuffer>(indices);
    }

    void VulkanBufferPool::CreateDescriptorSetLayout()
    {
        VulkanResources* vr = VulkanBase::GetResources();

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

        VkResult result = vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayout);
        DK_CORE_ASSERT(!result);
    }

    void VulkanBufferPool::CreateDescriptorPool()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        // std::array<VkDescriptorPoolSize, 2> poolSizes{};
        // poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // poolSizes[0].descriptorCount = static_cast<uint32_t>(vr->imageCount);
        // poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // poolSizes[1].descriptorCount = static_cast<uint32_t>(vr->imageCount) * 2; // imgui requires * 2 for extra texture

        // VkDescriptorPoolCreateInfo poolInfo{};
        // poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        // poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        // poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        // poolInfo.pPoolSizes = poolSizes.data();
        // poolInfo.maxSets = static_cast<uint32_t>(vr->imageCount) * 2;  // imgui requires * 2 for extra texture

        VkDescriptorPoolSize poolSizes[] = {
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

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * ((int)(sizeof(poolSizes) / sizeof(*(poolSizes))));
        poolInfo.poolSizeCount = (uint32_t)((int)(sizeof(poolSizes) / sizeof(*(poolSizes))));
        poolInfo.pPoolSizes = poolSizes;

        VkResult result = vkCreateDescriptorPool(vr->device, &poolInfo, nullptr, &vr->descriptorPool);
        DK_CORE_ASSERT(!result);

    }

    void VulkanBufferPool::CreateDescriptorSets()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        std::vector<VkDescriptorSetLayout> layouts(vr->imageCount, vr->descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = vr->descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(vr->imageCount);
        allocInfo.pSetLayouts = layouts.data();

        VkResult result = vkAllocateDescriptorSets(vr->device, &allocInfo, s_DescriptorSets.data());
        DK_CORE_ASSERT(!result);

        for (size_t i = 0; i < vr->imageCount; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = s_UniformBuffers[i]->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = VulkanTexturePool::GetViewportTexture()->GetImageView();
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

            vkUpdateDescriptorSets(vr->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void VulkanBufferPool::UpdateUniformBuffer(uint32_t currentImage)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.projection = glm::perspective(glm::radians(45.0f), vr->imageExtent.width / (float)vr->imageExtent.height, 0.1f, 10.0f);
        ubo.projection[1][1] *= -1;

        s_UniformBuffers[currentImage]->CopyTo(&ubo, sizeof(ubo));
    }

    void VulkanBufferPool::CleanUp()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        vkDestroyBuffer(vr->device, s_VertexBuffer->GetBuffer(), nullptr);
        vkFreeMemory(vr->device, s_VertexBuffer->GetMemory(), nullptr);

        vkDestroyBuffer(vr->device, s_IndexBuffer->GetBuffer(), nullptr);
        vkFreeMemory(vr->device, s_IndexBuffer->GetMemory(), nullptr);

        for (size_t i = 0; i < vr->imageCount; i++)
        {
            vkDestroyBuffer(vr->device, s_UniformBuffers[i]->GetBuffer(), nullptr);
            vkFreeMemory(vr->device, s_UniformBuffers[i]->GetMemory(), nullptr);
        }

        vkDestroyDescriptorPool(vr->device, vr->descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayout, nullptr);
    }

}
