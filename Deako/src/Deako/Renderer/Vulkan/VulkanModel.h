#pragma once


#include "VulkanTexture.h"
#include "VulkanMaterial.h"
#include "Deako/Asset/Asset.h"

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    #define MAX_NUM_JOINTS 128u

    const std::vector<std::string> supportedGLTFExts = {
        "KHR_texture_basisu",
        "KHR_materials_pbrSpecularGlossiness",
        "KHR_materials_unlit",
        "KHR_materials_emissive_strength"
    };

    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;
        bool valid{ false };

        BoundingBox() {}
        BoundingBox(glm::vec3 min, glm::vec3 max);
        BoundingBox GetAABB(glm::mat4 m);
    };

    struct Primitive
    {
        uint32_t firstIndex;
        uint32_t indexCount;
        uint32_t vertexCount;

        Material& material;
        bool hasIndices;
        BoundingBox boundingBox;

        Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material& material);
        void SetBoundingBox(glm::vec3 min, glm::vec3 max);
    };

    struct Mesh
    {
        std::vector<Primitive*> primitives;
        BoundingBox boundingBox;
        BoundingBox aaBoundingBox;

        struct UniformBuffer
        {
            VulkanBuffer::AllocatedBuffer buffer;
            VkDescriptorBufferInfo descriptor;
            VkDescriptorSet descriptorSet;
            void* mapped;
        } uniform;

        struct UniformBlock
        {
            glm::mat4 matrix;
            glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
            uint32_t jointcount{ 0 };
        } uniformBlock;

        Mesh(glm::mat4 matrix);
        ~Mesh();
        void SetBoundingBox(glm::vec3 min, glm::vec3 max);
    };

    struct Skin;

    struct Node
    {
        Node* parent;
        uint32_t index;
        std::vector<Node*> children;

        glm::mat4 matrix;
        std::string name;
        Mesh* mesh;
        Skin* skin;
        int32_t skinIndex{ -1 };

        glm::vec3 translation{};
        glm::vec3 scale{ 1.0f };
        glm::quat rotation{};

        BoundingBox boundingVH;
        BoundingBox aaBoundingBox;

        bool useCachedMatrix{ false };
        glm::mat4 cachedLocalMatrix{ glm::mat4(1.0f) };
        glm::mat4 cachedMatrix{ glm::mat4(1.0f) };

        void Update();
        ~Node();
        glm::mat4 LocalMatrix();
        glm::mat4 GetMatrix();
    };

    struct AnimationChannel
    {
        enum PathType { TRANSLATION, ROTATION, SCALE };
        PathType path;
        Node* node;
        uint32_t samplerIndex;
    };

    struct AnimationSampler
    {
        enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
        InterpolationType interpolation;
        std::vector<float> inputs;
        std::vector<glm::vec4> outputsVec4;
        std::vector<float> outputs;

        glm::vec4 CubicSplineInterpolation(size_t index, float time, uint32_t stride);
        void Translate(size_t index, float time, Node* node);
        void Scale(size_t index, float time, Node* node);
        void Rotate(size_t index, float time, Node* node);
    };

    struct Animation
    {
        std::string name;
        std::vector<AnimationSampler> samplers;
        std::vector<AnimationChannel> channels;

        float start = std::numeric_limits<float>::max();
        float end = std::numeric_limits<float>::min();
    };

    struct Skin
    {
        std::string name;
        Node* skeletonRoot = nullptr;
        std::vector<glm::mat4> inverseBindMatrices;
        std::vector<Node*> joints;
    };

    struct Model : public Asset
    {
        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv0;
            glm::vec2 uv1;
            glm::uvec4 joint0;
            glm::vec4 weight0;
            glm::vec4 color;
        };

        struct LoaderData
        {
            Buffer buffer;
            uint32_t count = 0;
            uint32_t position = 0;
        } vertexData, indexData;

        VulkanBuffer::AllocatedBuffer vertices;
        VulkanBuffer::AllocatedBuffer indices;

        std::filesystem::path path;

        std::vector<Node*> nodes;
        std::vector<Node*> linearNodes;

        std::vector<Ref<Material>> materials;
        std::vector<Animation> animations;
        std::vector<Skin*> skins;
        std::vector<std::string> extensions;

        struct Dimensions
        {
            glm::vec3 min = glm::vec3(FLT_MAX);
            glm::vec3 max = glm::vec3(-FLT_MAX);
        } dimensions;

        glm::mat4 aaBoundingBox;

        Model() = default;
        virtual void Destroy() override;

        void Draw(VkCommandBuffer commandBuffer);
        void DrawNode(Node* node, VkCommandBuffer commandBuffer);
        void UpdateAnimation(uint32_t index, float time);

        void SetMaterials(const std::vector<Ref<Material>>& materials);
        void SetVertices();
        void SetIndices();

        void DetermineDimensions();
        void CalculateBoundingBox(Node* node, Node* parent);

        static AssetType GetStaticType() { return AssetType::Model; }
        virtual AssetType GetType() const override { return GetStaticType(); }
    };

    void RenderNode(Node* node, VkCommandBuffer commandBuffer, Material::AlphaMode alphaMode, uint32_t dynamicOffset);

}
