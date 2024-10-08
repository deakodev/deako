#include "PrefabHandler.h"
#include "dkpch.h"

#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Mesh/MeshHandler.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"

#include "System/MacOS/MacUtils.h" 

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include <basisu_transcoder.h>

namespace Deako {

    using PrefabParseFunction = std::function<Ref<Prefab>(AssetHandle, PrefabMetadata)>;
    static std::map<PrefabType, PrefabParseFunction> s_PrefabImportFunctions = {
        { PrefabType::GLTF, PrefabHandler::ParseGLTF },
    };

    void PrefabHandler::Init()
    {
    }

    void PrefabHandler::CleanUp()
    {
    }

    void PrefabHandler::OpenPrefab()
    {
        std::filesystem::path prefabPath = MacUtils::File::Open("gltf", "Import Prefab");
        ImportPrefab(prefabPath);
    }

    Ref<Prefab> PrefabHandler::ImportPrefab(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Prefab;
        metadata.assetPath = path;

        Ref<Prefab> prefab = ImportPrefab(handle, metadata);
        return prefab;
    }

    Ref<Prefab> PrefabHandler::ImportPrefab(AssetHandle handle, AssetMetadata& metadata)
    {
        DK_CORE_INFO("Importing Prefab <{0}>", metadata.assetPath.filename().string());

        PrefabMetadata prefabMetadata;
        prefabMetadata.assetPath = metadata.assetPath;
        prefabMetadata.assetType = metadata.assetType;
        prefabMetadata.prefabType = PrefabType::GLTF; // TODO: add a switch

        auto it = s_PrefabImportFunctions.find(prefabMetadata.prefabType);
        if (it == s_PrefabImportFunctions.end())
        {
            DK_CORE_ERROR("No parse function available for prefab type: {0}", (uint16_t)prefabMetadata.prefabType);
            return nullptr;
        }

        Ref<Prefab> prefab = it->second(handle, prefabMetadata);

        if (prefab)
        {
            prefab->m_Handle = handle;

            Ref<ProjectAssetPool> projectAssetPool = ProjectAssetPool::Get();

            std::string assetName = metadata.assetPath.filename().string();
            assetName[0] = std::toupper(assetName[0]);
            metadata.assetName = assetName;
            projectAssetPool->AddAssetToPool(prefab, metadata);

            metadata.parentAssetHandle = prefab->m_Handle;
            metadata.assetType = AssetType::Model;
            metadata.assetName = "Mesh <" + metadata.assetPath.filename().string() + ">";
            projectAssetPool->AddAssetToPool(prefab->model, metadata);

            metadata.assetType = AssetType::Texture2D;
            metadata.assetName = "Texture2D <" + metadata.assetPath.filename().string() + ">";
            for (auto& [handle, texture] : prefab->textures)
                projectAssetPool->AddAssetToPool(texture, metadata);

            metadata.assetType = AssetType::Material;
            metadata.assetName = "Material <" + metadata.assetPath.filename().string() + ">";
            for (auto& [handle, material] : prefab->materials)
                projectAssetPool->AddAssetToPool(material, metadata);
        }

        return prefab;
    }

    // custom loading ktx textures function used with tinyglTF
    bool LoadImageDataFunc(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData)
    {
        if (image->uri.find_last_of(".") != std::string::npos)
        {    // ktx files will be handled by our own code
            if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx2")
                return true;
        }

        return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
    }

    Ref<Prefab> PrefabHandler::ParseGLTF(AssetHandle handle, PrefabMetadata metadata)
    {
        bool binary = false;
        if (metadata.assetPath.extension().string() == ".glb") binary = true;

        tinygltf::Model tinyModel;
        tinygltf::TinyGLTF gltfContext;

        std::string error;
        std::string warning;

        gltfContext.SetImageLoader(LoadImageDataFunc, nullptr);

        bool fileLoaded = binary ?
            gltfContext.LoadBinaryFromFile(&tinyModel, &error, &warning, metadata.assetPath.string()) :
            gltfContext.LoadASCIIFromFile(&tinyModel, &error, &warning, metadata.assetPath.string());

        if (fileLoaded)
        {
            Ref<Prefab> prefab = CreateRef<Prefab>();

            prefab->model = CreateRef<Model>();
            AssetMetadata modelMetadata;
            modelMetadata.assetType = AssetType::Model;

            prefab->model->extensions = tinyModel.extensionsUsed;
            for (auto& extension : prefab->model->extensions)
            {   // if model uses basis universal compressed textures, we need to transcode them
                // So we need to initialize that transcoder once
                if (extension == "KHR_texture_basisu")
                {
                    DK_CORE_INFO("Model uses KHR_texture_basisu, initializing basisu transcoder");
                    basist::basisu_transcoder_init();
                }
            }

            std::vector<TextureSampler> textureSamplers;
            textureSamplers.reserve(tinyModel.samplers.size());
            for (tinygltf::Sampler tinySampler : tinyModel.samplers)
            {   // construct a TextureSampler in place and get a reference to it
                TextureSampler& sampler = textureSamplers.emplace_back();
                sampler.SetFilterModes(tinySampler.minFilter, tinySampler.magFilter);
                sampler.SetWrapModes(tinySampler.wrapS, tinySampler.wrapT);
                sampler.addressModeW = sampler.addressModeV;
            }

            std::vector<Ref<Texture2D>> textures;
            textures.reserve(tinyModel.textures.size());
            for (tinygltf::Texture& tinyTexture : tinyModel.textures)
            {
                AssetMetadata textureMetadata;
                textureMetadata.assetType = AssetType::Texture2D;
                Ref<Texture2D> texture = CreateRef<Texture2D>();

                TextureSampler& sampler = textureSamplers[tinyTexture.sampler];

                int source = tinyTexture.source;
                if (tinyTexture.extensions.find("KHR_texture_basisu") != tinyTexture.extensions.end())
                {    // if a texture uses KHR_texture_basisu, get source index from extension structure
                    auto ext = tinyTexture.extensions.find("KHR_texture_basisu");
                    auto value = ext->second.Get("source");
                    source = value.Get<int>();
                }

                tinygltf::Image& tinyImage = tinyModel.images[source];
                if (tinyImage.uri.substr(tinyImage.uri.find_last_of(".") + 1) == "ktx2")
                {
                    std::ifstream file(tinyImage.uri, std::ios::binary | std::ios::in | std::ios::ate);
                    DK_CORE_ASSERT(file.is_open(), "Failed to open texture file!");

                    Buffer buffer(file.tellg());

                    file.seekg(0, std::ios::beg);
                    file.read(reinterpret_cast<char*>(buffer.data), buffer.size);

                    texture->GenerateFromGLTF(tinyImage, buffer, sampler);
                }
                else
                {
                    Buffer buffer; // empty buffer
                    texture->GenerateFromGLTF(tinyImage, buffer, sampler);
                }

                textures.emplace_back(texture);
                prefab->textures[texture->m_Handle] = texture;
            }

            std::vector<Ref<Material>> materials;
            materials.reserve(tinyModel.materials.size());
            for (tinygltf::Material& tinyMaterial : tinyModel.materials)
            {
                AssetMetadata materialMetadata;
                materialMetadata.assetType = AssetType::Material;
                Ref<Material> material = CreateRef<Material>(tinyMaterial, textures);

                material->index = static_cast<uint32_t>(materials.size());

                materials.emplace_back(material);
                prefab->materials[material->m_Handle] = material;
            }

            // push a default material at the end of the list for meshes with no material assigned
            static Ref<Material> material = CreateRef<Material>();
            materials.emplace_back(material);
            prefab->materials[material->m_Handle] = material;

            prefab->model->SetMaterials(materials);

            ///// MODEL
            const tinygltf::Scene& tinyScene = tinyModel.scenes[tinyModel.defaultScene > -1 ? tinyModel.defaultScene : 0];

            // get vertex and index buffer sizes up-front
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
                MeshHandler::GetNodeProps(tinyModel.nodes[tinyScene.nodes[i]], tinyModel, prefab->model);

            prefab->model->vertexData.buffer.Allocate(sizeof(Model::Vertex) * prefab->model->vertexData.count);
            prefab->model->indexData.buffer.Allocate(sizeof(uint32_t) * prefab->model->indexData.count);

            DK_CORE_ASSERT(prefab->model->vertexData.buffer.size > 0);

            // TODO: scene handling with no default scene
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
            {
                const tinygltf::Node tinyNode = tinyModel.nodes[tinyScene.nodes[i]];
                MeshHandler::LoadNode(nullptr, tinyNode, tinyScene.nodes[i], tinyModel, prefab->model, 1.0f);
            }

            if (tinyModel.animations.size() > 0)
                MeshHandler::LoadAnimations(tinyModel, prefab->model);

            MeshHandler::LoadSkins(tinyModel, prefab->model);

            for (auto linearNode : prefab->model->linearNodes)
            {   // assign skins
                if (linearNode->skinIndex > -1) linearNode->skin = prefab->model->skins[linearNode->skinIndex];
                // initial pose
                if (linearNode->mesh) linearNode->Update();
            }

            // vulkan side
            prefab->model->SetVertices();
            prefab->model->SetIndices();
            prefab->model->DetermineDimensions();

            prefab->model->vertexData.buffer.Release();
            prefab->model->indexData.buffer.Release();

            for (auto& ext : prefab->model->extensions)
            {   // check and list unsupported extensions
                if (std::find(supportedGLTFExts.begin(), supportedGLTFExts.end(), ext) == supportedGLTFExts.end())
                    DK_CORE_WARN("Unsupported extension {0}. Model may not display as intended.", ext);
            }

            return prefab;
        }

        return nullptr;
    }

}
