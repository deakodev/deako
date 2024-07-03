#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace Deako {

    struct Vertex
    {
        glm::vec2 position;
        glm::vec3 color;

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
    };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    class Buffer
    {
    public:
        Buffer(VkDevice device);

        virtual void SetInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        virtual void CopyStaging(VkBuffer stagingBuffer, VkBuffer receivingBuffer, VkDeviceSize size);

        virtual void Bind(VkDeviceSize offset = 0);
        virtual void Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        virtual void Unmap();
        virtual void CopyTo(const void* data, VkDeviceSize size);

        VkBuffer& GetBuffer() { return m_Buffer; }
        VkDeviceMemory& GetMemory() { return m_Memory; }
        void*& Mapped() { return m_Mapped; }

    private:
        virtual uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        VkDevice m_Device{ VK_NULL_HANDLE };
        VkBuffer m_Buffer{ VK_NULL_HANDLE };
        VkDeviceMemory m_Memory{ VK_NULL_HANDLE };
        void* m_Mapped = nullptr;
    };

    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(VkDevice device, const std::vector<Vertex>& vertices);

        const std::vector<Vertex>& GetVertices() { return m_Vertices; }

    private:
        std::vector<Vertex> m_Vertices;
    };

    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(VkDevice device, const std::vector<uint16_t>& indices);

        const std::vector<uint16_t>& GetIndices() { return m_Indices; }

    private:
        std::vector<uint16_t> m_Indices;
    };

    class VulkanBufferPool
    {
    public:
        static void CreateUniformBuffers();
        static void CreateVertexBuffers();
        static void CreateIndexBuffer();
        static void CreateDescriptorSetLayout();
        static void CreateDescriptorPool();
        static void CreateDescriptorSets();
        static void CleanUp();

        static const Ref<VertexBuffer>& GetVertexBuffer() { return s_VertexBuffer; }
        static const Ref<IndexBuffer>& GetIndexBuffer() { return s_IndexBuffer; }
        static VkDescriptorSetLayout& GetDescriptorSetLayout() { return s_DescriptorSetLayout; }
        static VkDescriptorSet& GetDescriptorSet(uint32_t currentImage) { return s_DescriptorSets[currentImage]; }

        static void UpdateUniformBuffer(uint32_t currentImage);

    private:
        static VkDevice s_Device;

        static VkDescriptorSetLayout s_DescriptorSetLayout;
        static VkDescriptorPool s_DescriptorPool;
        static std::array<VkDescriptorSet, VulkanBase::MAX_FRAMES_IN_FLIGHT> s_DescriptorSets;

        static std::array<Ref<Buffer>, VulkanBase::MAX_FRAMES_IN_FLIGHT> s_UniformBuffers;
        static Ref<VertexBuffer> s_VertexBuffer;
        static Ref<IndexBuffer> s_IndexBuffer;
    };

}


