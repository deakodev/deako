#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace Deako {

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
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
        VkBuffer& GetBuffer() { return m_Buffer; }
        VkDeviceMemory& GetMemory() { return m_Memory; }

        virtual void SetInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        virtual void CopyStaging(VkBuffer stagingBuffer, VkBuffer receivingBuffer, VkDeviceSize size);

        virtual void Bind(VkDeviceSize offset = 0);
        virtual void Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        virtual void Unmap();
        virtual void CopyTo(const void* data, VkDeviceSize size);

        void*& Mapped() { return m_Mapped; }

        static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        VkBuffer m_Buffer{ VK_NULL_HANDLE };
        VkDeviceMemory m_Memory{ VK_NULL_HANDLE };
        void* m_Mapped = nullptr;
    };

    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const std::vector<Vertex>& vertices);

        const std::vector<Vertex>& GetVertices() { return m_Vertices; }

    private:
        std::vector<Vertex> m_Vertices;
    };

    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const std::vector<uint16_t>& indices);

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
        static VkDescriptorSet& GetDescriptorSet(uint32_t currentImage) { return s_DescriptorSets[currentImage]; }

        static void UpdateUniformBuffer(uint32_t currentImage);

    private:
        static std::array<VkDescriptorSet, 2> s_DescriptorSets;
        static std::array<Ref<Buffer>, 2> s_UniformBuffers;
        static Ref<VertexBuffer> s_VertexBuffer;
        static Ref<IndexBuffer> s_IndexBuffer;
    };

}


