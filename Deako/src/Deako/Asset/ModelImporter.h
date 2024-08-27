#pragma once

#include "Asset.h"

#include "Deako/Renderer/Vulkan/VulkanModel.h"

namespace Deako {

    class ModelImporter
    {
    public:
        static Ref<Model> ImportModel(const AssetMetadata& metadata);

        static void LoadTextureSamplers(tinygltf::Model& tinyModel, Ref<Model> model);
        static void LoadTextures(tinygltf::Model& tinyModel, Ref<Model> model);
        static void LoadMaterials(tinygltf::Model& tinyModel, Ref<Model> model);
        static void LoadNode(Node* parent, const tinygltf::Node& tinyNode, uint32_t nodeIndex, const tinygltf::Model& tinyModel, Ref<Model> model, float globalscale);
        static void LoadAnimations(tinygltf::Model& tinyModel, Ref<Model> model);
        static void LoadSkins(tinygltf::Model& tinyModel, Ref<Model> model);

        static void GetNodeProps(const tinygltf::Node& tinyNode, const tinygltf::Model& tinyModel, Ref<Model> model);
        static Node* NodeFromIndex(uint32_t index, Ref<Model> model);
        static Node* FindNode(Node* parent, uint32_t index);
    };

}
