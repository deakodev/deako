#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <functional>

namespace Deako {

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texCoord;

        bool operator==(const Vertex& other) const
        {
            return position == other.position &&
                color == other.color &&
                texCoord == other.texCoord;
        }
    };

    struct UniformBufferObject
    {
        glm::mat4 viewProjection;
    };

    struct InstanceData
    {
        glm::vec3 position;
        glm::vec3 rotation;
        float scale{ 0.0f };
        uint32_t textureIndex{ 0 };
    };

    class Buffer
    {
    public:
        Buffer(VkDeviceSize bufferSize = 0, size_t dataSize = 0);

        void SetBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

        void CopyToThis(VkBuffer stagingBuffer);
        void CopyToStaging(const void* data);
        void Map();
        void Unmap();
        void Destroy();

        VkBuffer& GetBuffer() { return m_Buffer; }
        VkDeviceMemory& GetMemory() { return m_Memory; }
        size_t GetDataSize() { return m_DataSize; }
        void*& Mapped() { return m_Mapped; }

        static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    protected:
        VkBuffer                m_Buffer{ VK_NULL_HANDLE };
        VkDeviceMemory          m_Memory{ VK_NULL_HANDLE };
        VkDeviceSize            m_BufferSize{ 0 };
        size_t                  m_DataSize{ 0 };
        void* /*             */ m_Mapped{ nullptr };

        static Ref<VulkanResources> s_VR;
        static Ref<VulkanSettings> s_VS;
    };

    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const std::vector<Vertex>& vertices);
    };

    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const std::vector<uint32_t>& indices);
    };

    class InstanceBuffer : public Buffer
    {
    public:
        InstanceBuffer(const std::vector<InstanceData>& instanceData);
    };

    class Model
    {
    public:
        void LoadFromFile(const std::string& path);

        const std::vector<Vertex>& GetVertices() { return m_Vertices; }
        const std::vector<uint32_t>& GetIndices() { return m_Indices; }

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
    };

    class BufferPool
    {
    public:
        static void CreateUniformBuffers();
        static void CreateDescriptorSetLayout();
        static void CreateDescriptorPool();
        static void CreateDescriptorSets();
        static void CleanUp();

        // static VertexBuffer& GetModelVertexBuffer() { return s_ModelVertexBuffer; }
        // static IndexBuffer& GetModelIndexBuffer() { return s_ModelIndexBuffer; }
        // static InstanceBuffer& GetModelInstanceBuffer() { return s_ModelInstanceBuffer; }

        static void PushModel(const Ref<Model>& model) { s_Models.push_back(model); }
        static const Ref<Model>& GetModel() { return s_Models[0]; }
        static std::vector<Ref<Model>>& GetModels() { return s_Models; }

        static VkDescriptorSet& GetDescriptorSet(uint32_t currentImage) { return s_DescriptorSets[currentImage]; }

        static void UpdateUniformBuffer(const glm::mat4& viewProjection);

    private:
        static std::array<VkDescriptorSet, 2> s_DescriptorSets;
        static std::array<Ref<Buffer>, 2> s_UniformBuffers;

        static std::vector<Ref<Model>> s_Models;

        static Ref<VulkanResources> s_VR;
        static Ref<VulkanSettings> s_VS;
    };

}


