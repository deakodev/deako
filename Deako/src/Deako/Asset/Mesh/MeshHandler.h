#pragma once

#include "Deako/Renderer/Vulkan/VulkanModel.h"

namespace Deako {

    class MeshHandler
    {
    public:
        static void Init();
        static void CleanUp();

        static Ref<Model> ImportMesh(AssetHandle handle, AssetMetadata& metadata);

        static void LoadNode(Node* parent, const tinygltf::Node& tinyNode, uint32_t nodeIndex, const tinygltf::Model& tinyModel, Ref<Model> model, float globalscale);
        static void LoadAnimations(tinygltf::Model& tinyModel, Ref<Model> model);
        static void LoadSkins(tinygltf::Model& tinyModel, Ref<Model> model);

        static void GetNodeProps(const tinygltf::Node& tinyNode, const tinygltf::Model& tinyModel, Ref<Model> model);
        static Node* NodeFromIndex(uint32_t index, Ref<Model> model);
        static Node* FindNode(Node* parent, uint32_t index);

    };



}
