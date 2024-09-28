#include "PrefabImporter.h"
#include "dkpch.h"

#include "Deako/Asset/ModelImporter.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include <basisu_transcoder.h>

namespace Deako {

    using PrefabImportFunction = std::function<Ref<Prefab>(AssetHandle, PrefabMetadata)>;

    static std::map <PrefabType, PrefabImportFunction> s_PrefabImportFunctions = {
        { PrefabType::GLTF, PrefabImporter::ImportGLTF },
    };

    Ref<Prefab> PrefabImporter::ImportPrefab(AssetHandle handle, AssetMetadata metadata)
    {
        PrefabMetadata prefabMetadata;
        prefabMetadata.assetPath = metadata.assetPath;
        prefabMetadata.assetType = metadata.assetType;
        prefabMetadata.prefabType = PrefabType::GLTF; // TODO: AddAsset a switch later if needed

        auto it = s_PrefabImportFunctions.find(prefabMetadata.prefabType);
        if (it == s_PrefabImportFunctions.end())
        {
            DK_CORE_ERROR("No import function available for prefab type: {0}", (uint16_t)prefabMetadata.prefabType);
            return nullptr;
        }

        return it->second(handle, prefabMetadata);
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

    Ref<Prefab> PrefabImporter::ImportGLTF(AssetHandle handle, PrefabMetadata metadata)
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
            for (tinygltf::Sampler tinySampler : tinyModel.samplers)
            {
                TextureSampler sampler{};
                sampler.SetFilterModes(tinySampler.minFilter, tinySampler.magFilter);
                sampler.SetWrapModes(tinySampler.wrapS, tinySampler.wrapT);
                sampler.addressModeW = sampler.addressModeV;

                textureSamplers.push_back(sampler);
            }

            std::vector<Ref<Texture2D>> textures;
            for (tinygltf::Texture& tinyTexture : tinyModel.textures)
            {
                int source = tinyTexture.source;
                if (tinyTexture.extensions.find("KHR_texture_basisu") != tinyTexture.extensions.end())
                {    // if a texture uses KHR_texture_basisu, get source index from extension structure
                    auto ext = tinyTexture.extensions.find("KHR_texture_basisu");
                    auto value = ext->second.Get("source");
                    source = value.Get<int>();
                }

                tinygltf::Image tinyImage = tinyModel.images[source];

                TextureSampler sampler = textureSamplers[tinyTexture.sampler];

                Ref<Texture2D> texture = CreateRef<Texture2D>();
                AssetMetadata textureMetadata;
                textureMetadata.assetType = AssetType::Texture2D;

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

                textures.push_back(texture);
                prefab->textures[texture->m_Handle] = texture;

                AssetPool::AddAsset(texture, textureMetadata);
            }

            std::vector<Ref<Material>> materials;
            for (tinygltf::Material& tinyMaterial : tinyModel.materials)
            {
                Ref<Material> material = CreateRef<Material>(tinyMaterial, textures);
                AssetMetadata materialMetadata;
                materialMetadata.assetType = AssetType::Material;

                material->index = static_cast<uint32_t>(materials.size());

                materials.push_back(material);
                prefab->materials[material->m_Handle] = material;

                AssetPool::AddAsset(material, materialMetadata);
            }

            // push a default material at the end of the list for meshes with no material assigned
            // TODO: only need one for all meshes??
            Ref<Material> material = CreateRef<Material>();
            AssetMetadata materialMetadata;
            materialMetadata.assetType = AssetType::Material;

            prefab->materials[material->m_Handle] = material;

            AssetPool::AddAsset(material, materialMetadata);

            materials.push_back(material);
            prefab->model->SetMaterials(materials);

            ///// MODEL
            const tinygltf::Scene& tinyScene = tinyModel.scenes[tinyModel.defaultScene > -1 ? tinyModel.defaultScene : 0];

            // get vertex and index buffer sizes up-front
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
                ModelImporter::GetNodeProps(tinyModel.nodes[tinyScene.nodes[i]], tinyModel, prefab->model);

            prefab->model->vertexData.buffer.Allocate(sizeof(Model::Vertex) * prefab->model->vertexData.count);
            prefab->model->indexData.buffer.Allocate(sizeof(uint32_t) * prefab->model->indexData.count);

            DK_CORE_ASSERT(prefab->model->vertexData.buffer.size > 0);

            // TODO: scene handling with no default scene
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
            {
                const tinygltf::Node tinyNode = tinyModel.nodes[tinyScene.nodes[i]];
                ModelImporter::LoadNode(nullptr, tinyNode, tinyScene.nodes[i], tinyModel, prefab->model, 1.0f);
            }

            if (tinyModel.animations.size() > 0)
                ModelImporter::LoadAnimations(tinyModel, prefab->model);

            ModelImporter::LoadSkins(tinyModel, prefab->model);

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

            AssetPool::AddAsset(std::static_pointer_cast<Asset>(prefab->model), modelMetadata);

            AssetPool::AddAsset(std::static_pointer_cast<Asset>(prefab), metadata);

            return prefab;
        }

        return nullptr;
    }

}
