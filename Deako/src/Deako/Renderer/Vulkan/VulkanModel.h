#pragma once


#include "VulkanTexture.h"
#include "VulkanMaterial.h"
#include "Deako/Asset/Asset.h"

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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
        DkVec3 min;
        DkVec3 max;
        bool valid{ false };

        BoundingBox() {}
        BoundingBox(DkVec3 min, DkVec3 max);
        BoundingBox GetAABB(DkMat4 m);
    };

    struct Primitive
    {
        DkU32 firstIndex;
        DkU32 indexCount;
        DkU32 vertexCount;

        Material& material;
        bool hasIndices;
        BoundingBox boundingBox;

        Primitive(DkU32 firstIndex, DkU32 indexCount, DkU32 vertexCount, Material& material);
        void SetBoundingBox(DkVec3 min, DkVec3 max);
    };

    struct Mesh
    {
        std::vector<Primitive*> primitives;
        BoundingBox boundingBox;
        BoundingBox aaBoundingBox;

        struct UniformBuffer
        {
            AllocatedBuffer buffer;
            VkDescriptorBufferInfo descriptor;
            VkDescriptorSet descriptorSet;
            void* mapped;
        } uniform;

        struct UniformBlock
        {
            DkMat4 matrix;
            DkMat4 jointMatrix[MAX_NUM_JOINTS]{};
            DkU32 jointcount{ 0 };
        } uniformBlock;

        Mesh(DkMat4 matrix);
        ~Mesh();
        void SetBoundingBox(DkVec3 min, DkVec3 max);
    };

    struct Skin;

    struct Node
    {
        Node* parent;
        DkU32 index;
        std::vector<Node*> children;

        DkMat4 matrix;
        std::string name;
        Mesh* mesh;
        Skin* skin;
        DkS32 skinIndex{ -1 };

        DkVec3 translation{};
        DkVec3 scale{ 1.0f };
        glm::quat rotation{};

        BoundingBox boundingVH;
        BoundingBox aaBoundingBox;

        bool useCachedMatrix{ false };
        DkMat4 cachedLocalMatrix{ DkMat4(1.0f) };
        DkMat4 cachedMatrix{ DkMat4(1.0f) };

        void Update();
        ~Node();
        DkMat4 LocalMatrix();
        DkMat4 GetMatrix();
    };

    struct AnimationChannel
    {
        enum PathType { TRANSLATION, ROTATION, SCALE };
        PathType path;
        Node* node;
        DkU32 samplerIndex;
    };

    struct AnimationSampler
    {
        enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
        InterpolationType interpolation;
        std::vector<DkF32> inputs;
        std::vector<DkVec4> outputsVec4;
        std::vector<DkF32> outputs;

        DkVec4 CubicSplineInterpolation(size_t index, DkF32 time, DkU32 stride);
        void Translate(size_t index, DkF32 time, Node* node);
        void Scale(size_t index, DkF32 time, Node* node);
        void Rotate(size_t index, DkF32 time, Node* node);
    };

    struct Animation
    {
        std::string name;
        std::vector<AnimationSampler> samplers;
        std::vector<AnimationChannel> channels;

        DkF32 start = std::numeric_limits<DkF32>::max();
        DkF32 end = std::numeric_limits<DkF32>::min();
    };

    struct Skin
    {
        std::string name;
        Node* skeletonRoot = nullptr;
        std::vector<DkMat4> inverseBindMatrices;
        std::vector<Node*> joints;
    };

    struct Model : public Asset
    {
        struct Vertex
        {
            DkVec3 pos;
            DkVec3 normal;
            DkVec2 uv0;
            DkVec2 uv1;
            DkUVec4 joint0;
            DkVec4 weight0;
            DkVec4 color;
        };

        struct LoaderData
        {
            Buffer buffer;
            DkU32 count = 0;
            DkU32 position = 0;
        } vertexData, indexData;

        AllocatedBuffer vertices;
        AllocatedBuffer indices;

        std::filesystem::path path;

        std::vector<Node*> nodes;
        std::vector<Node*> linearNodes;

        std::vector<Ref<Material>> materials;
        std::vector<Animation> animations;
        std::vector<Skin*> skins;
        std::vector<std::string> extensions;

        struct Dimensions
        {
            DkVec3 min = DkVec3(FLT_MAX);
            DkVec3 max = DkVec3(-FLT_MAX);
        } dimensions;

        DkMat4 aaBoundingBox;

        Model() = default;
        virtual void Destroy() override;

        void Draw(VkCommandBuffer commandBuffer);
        void DrawNode(Node* node, VkCommandBuffer commandBuffer);
        void UpdateAnimation(DkU32 index, DkF32 time);

        void SetMaterials(const std::vector<Ref<Material>>& materials);
        void SetVertices();
        void SetIndices();

        void DetermineDimensions();
        void CalculateBoundingBox(Node* node, Node* parent);

        static AssetType GetStaticType() { return AssetType::Model; }
        virtual AssetType GetType() const override { return GetStaticType(); }
    };

    void RenderNode(Node* node, VkCommandBuffer commandBuffer, Material::AlphaMode alphaMode, DkU32 dynamicOffset);

}
