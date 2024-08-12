#pragma once

#include "VulkanTypes.h"
#include "VulkanTexture.h"
#include "VulkanMaterial.h"

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#define BASISU_HAVE_STD_TRIVIALLY_COPYABLE
#include <basisu_transcoder.h>

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
            AllocatedBuffer buffer;
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
        void SetDescriptorSet();
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

    struct Model
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

        struct LoaderInfo
        {
            uint32_t* indexBuffer;
            Vertex* vertexBuffer;
            size_t indexPos = 0;
            size_t vertexPos = 0;
        };

        AllocatedBuffer vertices;
        AllocatedBuffer indices;

        glm::mat4 aaBoundingBox;

        std::vector<Node*> nodes;
        std::vector<Node*> linearNodes;

        std::vector<Texture2D> textures;
        std::vector<TextureSampler> textureSamplers;
        std::vector<Material> materials;
        std::vector<Animation> animations;
        std::vector<Skin*> skins;
        std::vector<std::string> extensions;

        std::filesystem::path path;

        struct Dimensions
        {
            glm::vec3 min = glm::vec3(FLT_MAX);
            glm::vec3 max = glm::vec3(-FLT_MAX);
        } dimensions;

        void Draw(VkCommandBuffer commandBuffer);
        void DrawNode(Node* node, VkCommandBuffer commandBuffer);
        void UpdateAnimation(uint32_t index, float time);

        void LoadFromFile(float scale = 1.0f);
        void LoadTextureSamplers(tinygltf::Model& tinyModel);
        void LoadTextures(tinygltf::Model& tinyModel);
        void LoadMaterials(tinygltf::Model& tinyModel);
        void LoadNode(Node* parent, const tinygltf::Node& tinyNode, uint32_t nodeIndex, const tinygltf::Model& tinyModel, LoaderInfo& loaderInfo, float globalscale);
        void LoadAnimations(tinygltf::Model& tinyModel);
        void LoadSkins(tinygltf::Model& tinyModel);

        void GetNodeProps(const tinygltf::Node& tinyNode, const tinygltf::Model& tinyModel, size_t& vertexCount, size_t& indexCount);
        Node* NodeFromIndex(uint32_t index);
        Node* FindNode(Node* parent, uint32_t index);
        void GetSceneDimensions();
        void CalculateBoundingBox(Node* node, Node* parent);

        void Destroy();
    };

    void RenderNode(Node* node, VkCommandBuffer commandBuffer, Material::AlphaMode alphaMode);

}
