#include "VulkanBuffer.h"
#include "dkpch.h"

// TODO: temp
#include "Deako/Renderer/Renderer.h"

#include "VulkanCommand.h"
#include "VulkanTexture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION 
#include <tiny_obj_loader.h>

namespace std {

    template<>
    struct hash<Deako::Vertex>
    {
        size_t operator()(Deako::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.position) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
} // end std namespace

namespace Deako {

    std::vector<Ref<Model>> BufferPool::s_Models;
    std::array<VkDescriptorSet, 2> BufferPool::s_DescriptorSets;
    std::array<Ref<Buffer>, 2> BufferPool::s_UniformBuffers;
    Ref<VulkanResources> BufferPool::s_VR = VulkanBase::GetResources();
    Ref<VulkanResources> Buffer::s_VR = VulkanBase::GetResources();
    Ref<VulkanSettings> BufferPool::s_VS = VulkanBase::GetSettings();
    Ref<VulkanSettings> Buffer::s_VS = VulkanBase::GetSettings();

    Buffer::Buffer(VkDeviceSize bufferSize, size_t dataSize)
        :m_BufferSize(bufferSize), m_DataSize(dataSize)
    {
    }

    VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices)
        : Buffer(vertices.size() * sizeof(Vertex), vertices.size())
    {
        Buffer stagingBuffer{ m_BufferSize };
        stagingBuffer.SetBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyToStaging(vertices.data());
        stagingBuffer.Unmap();

        this->SetBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        this->CopyToThis(stagingBuffer.GetBuffer());

        vkDestroyBuffer(s_VR->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, stagingBuffer.GetMemory(), nullptr);
    }

    IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices)
        : Buffer(indices.size() * sizeof(uint32_t), indices.size())
    {
        Buffer stagingBuffer{ m_BufferSize };
        stagingBuffer.SetBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyToStaging(indices.data());
        stagingBuffer.Unmap();

        this->SetBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        this->CopyToThis(stagingBuffer.GetBuffer());

        vkDestroyBuffer(s_VR->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, stagingBuffer.GetMemory(), nullptr);
    }

    InstanceBuffer::InstanceBuffer(const std::vector<InstanceData>& instanceData)
        : Buffer(instanceData.size() * sizeof(InstanceData), instanceData.size())
    {
        Buffer stagingBuffer{ m_BufferSize };
        stagingBuffer.SetBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyToStaging(instanceData.data());
        stagingBuffer.Unmap();

        this->SetBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        this->CopyToThis(stagingBuffer.GetBuffer());

        vkDestroyBuffer(s_VR->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, stagingBuffer.GetMemory(), nullptr);
    }

    void Buffer::SetBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = m_BufferSize;
        // indicates for which purposes the data in the buffer is going to be used
        bufferInfo.usage = usage;
        // buffers can be owned by a specific queue family or be shared
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK_RESULT(vkCreateBuffer(s_VR->device, &bufferInfo, nullptr, &m_Buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(s_VR->device, m_Buffer, &memRequirements);

        // allocate the memory
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        VK_CHECK_RESULT(vkAllocateMemory(s_VR->device, &allocInfo, nullptr, &m_Memory));

        VK_CHECK_RESULT(vkBindBufferMemory(s_VR->device, m_Buffer, m_Memory, 0));
    }

    void Buffer::CopyToThis(VkBuffer stagingBuffer)
    {
        VkCommandBuffer commandBuffer = CommandPool::BeginSingleTimeCommands(s_VR->viewportCommandPool);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = m_BufferSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_Buffer, 1, &copyRegion);

        CommandPool::EndSingleTimeCommands(s_VR->viewportCommandPool, commandBuffer);
    }

    void Buffer::CopyToStaging(const void* data)
    {
        DK_CORE_ASSERT(m_Mapped);
        memcpy(m_Mapped, data, m_BufferSize);
    }

    void Buffer::Map()
    {
        VK_CHECK_RESULT(vkMapMemory(s_VR->device, m_Memory, 0, m_BufferSize, 0, &m_Mapped));
    }

    void Buffer::Unmap()
    {
        if (m_Mapped)
        {
            vkUnmapMemory(s_VR->device, m_Memory);
            m_Mapped = nullptr;
        }
    }

    void Buffer::Destroy()
    {
        if (m_Buffer)
            vkDestroyBuffer(s_VR->device, m_Buffer, nullptr);
        if (m_Memory)
            vkFreeMemory(s_VR->device, m_Memory, nullptr);
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
            s_UniformBuffers[i] = CreateRef<Buffer>(bufferSize);

            s_UniformBuffers[i]->SetBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            vkMapMemory(s_VR->device, s_UniformBuffers[i]->GetMemory(), 0, bufferSize, 0, &s_UniformBuffers[i]->Mapped());
        }
    }

    void BufferPool::CreateDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            // Binding 0 : Vertex shader uniform buffer
            VulkanInitializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1),
            // Binding 1 : Fragment shader combined sampler
            VulkanInitializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1),
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(setLayoutBindings);

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(s_VR->device, &layoutInfo, nullptr, &s_VR->descriptorSetLayout));
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

        VK_CHECK_RESULT(vkCreateDescriptorPool(s_VR->device, &poolInfo, nullptr, &s_VR->descriptorPool));
    }

    void BufferPool::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(s_VS->imageCount, s_VR->descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo =
            VulkanInitializers::DescriptorSetAllocateInfo(s_VR->descriptorPool, layouts.data(), static_cast<uint32_t>(s_VS->imageCount));

        VK_CHECK_RESULT(vkAllocateDescriptorSets(s_VR->device, &allocInfo, s_DescriptorSets.data()));

        for (size_t i = 0; i < s_VS->imageCount; i++)
        {
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            // Binding 0 : Vertex shader uniform buffer
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

            // Binding 1 : Fragment shader combined sampler
            const std::vector<Ref<Texture2D>>& textures = TexturePool::GetTexture2Ds();
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textures[0]->GetImageView();
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

    void BufferPool::CleanUp()
    {
        for (size_t i = 0; i < s_VS->imageCount; i++)
        {
            vkDestroyBuffer(s_VR->device, s_UniformBuffers[i]->GetBuffer(), nullptr);
            vkFreeMemory(s_VR->device, s_UniformBuffers[i]->GetMemory(), nullptr);
        }

        vkDestroyDescriptorPool(s_VR->device, s_VR->descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(s_VR->device, s_VR->descriptorSetLayout, nullptr);
    }


    void BufferPool::UpdateUniformBuffer(const glm::mat4& viewProjection)
    {
        uint32_t currentFrame = VulkanBase::GetCurrentFrame();

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        static UniformBufferObject ubo{};
        // TODO: glm::rotate temp 
        ubo.viewProjection = viewProjection * glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.viewProjection = viewProjection;

        DK_CORE_ASSERT(s_UniformBuffers[currentFrame]->Mapped());
        memcpy(s_UniformBuffers[currentFrame]->Mapped(), &ubo, sizeof(UniformBufferObject));
    }

    void Model::LoadFromFile(const std::string& path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn, err;
        bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
        if (!success)
            throw std::runtime_error(warn + err);

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                    m_Vertices.push_back(vertex);
                }

                m_Indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

}
