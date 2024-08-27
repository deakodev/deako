#include "GLTFImporter.h"
#include "dkpch.h"

#include "AssetPoolWrapper.h"
#include "ModelImporter.h"

#include "Deako/Renderer/Vulkan/VulkanTexture.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#define BASISU_HAVE_STD_TRIVIALLY_COPYABLE
#include <basisu_transcoder.h>

namespace Deako {

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

    Ref<Model> GLTFImporter::ImportGLTF(const std::filesystem::path& path)
    {
        bool binary = false;
        if (path.extension().string() == ".glb") binary = true;

        tinygltf::Model tinyModel;
        tinygltf::TinyGLTF gltfContext;

        std::string error;
        std::string warning;

        gltfContext.SetImageLoader(LoadImageDataFunc, nullptr);

        bool fileLoaded = binary ?
            gltfContext.LoadBinaryFromFile(&tinyModel, &error, &warning, path.string()) :
            gltfContext.LoadASCIIFromFile(&tinyModel, &error, &warning, path.string());

        if (fileLoaded)
        {
            Ref<Model> model = CreateRef<Model>();

            model->extensions = tinyModel.extensionsUsed;
            for (auto& extension : model->extensions)
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

                AssetMetadata metadata;
                metadata.type = AssetType::Texture2D;

                if (tinyImage.uri.substr(tinyImage.uri.find_last_of(".") + 1) == "ktx2")
                {
                    std::ifstream file(path.string(), std::ios::binary | std::ios::in | std::ios::ate);
                    DK_CORE_ASSERT(file.is_open(), "Failed to open texture file!");

                    Buffer buffer(file.tellg());

                    file.seekg(0, std::ios::beg);
                    file.read(reinterpret_cast<char*>(buffer.data), buffer.size);

                    texture->GenerateFromGLTF(tinyImage, buffer, sampler);

                    metadata.path = tinyImage.uri;
                }
                else
                {
                    Buffer buffer; // empty buffer
                    texture->GenerateFromGLTF(tinyImage, buffer, sampler);

                    metadata.path = path;
                }

                Ref<Asset> asset = std::static_pointer_cast<Asset>(texture);
                AssetPool::AddAsset(asset, metadata);

                textures.push_back(texture);
            }

            std::vector<Ref<Material>> materials;
            for (tinygltf::Material& tinyMaterial : tinyModel.materials)
            {
                Ref<Material> material = CreateRef<Material>(tinyMaterial, textures);

                AssetMetadata metadata;
                metadata.path = path;
                metadata.type = AssetType::Material;

                Ref<Asset> asset = std::static_pointer_cast<Asset>(material);
                AssetPool::AddAsset(asset, metadata);

                material->index = static_cast<uint32_t>(materials.size());
                materials.push_back(material);
            }

            // push a default material at the end of the list for meshes with no material assigned
            // TODO: only need one for all meshes
            materials.push_back(CreateRef<Material>());

            model->SetMaterials(materials);

            ///// MODEL
            const tinygltf::Scene& tinyScene = tinyModel.scenes[tinyModel.defaultScene > -1 ? tinyModel.defaultScene : 0];

            // get vertex and index buffer sizes up-front
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
                ModelImporter::GetNodeProps(tinyModel.nodes[tinyScene.nodes[i]], tinyModel, model);

            model->vertexData.buffer.Allocate(sizeof(Model::Vertex) * model->vertexData.count);
            model->indexData.buffer.Allocate(sizeof(uint32_t) * model->indexData.count);

            DK_CORE_ASSERT(model->vertexData.buffer.size > 0);

            // TODO: scene handling with no default scene
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
            {
                const tinygltf::Node tinyNode = tinyModel.nodes[tinyScene.nodes[i]];
                ModelImporter::LoadNode(nullptr, tinyNode, tinyScene.nodes[i], tinyModel, model, 1.0f);
            }

            if (tinyModel.animations.size() > 0)
                ModelImporter::LoadAnimations(tinyModel, model);

            ModelImporter::LoadSkins(tinyModel, model);

            for (auto linearNode : model->linearNodes)
            {   // assign skins
                if (linearNode->skinIndex > -1) linearNode->skin = model->skins[linearNode->skinIndex];
                // initial pose
                if (linearNode->mesh) linearNode->Update();
            }

            // vulkan side
            model->SetVertices();
            model->SetIndices();
            model->DetermineDimensions();

            model->vertexData.buffer.Release();
            model->indexData.buffer.Release();

            for (auto& ext : model->extensions)
            {   // check and list unsupported extensions
                if (std::find(supportedGLTFExts.begin(), supportedGLTFExts.end(), ext) == supportedGLTFExts.end())
                    DK_CORE_WARN("Unsupported extension {0}. Model may not display as intended.", ext);
            }

            AssetMetadata metadata;
            metadata.path = path;
            metadata.type = AssetType::Model;

            Ref<Asset> asset = std::static_pointer_cast<Asset>(model);
            AssetPool::AddAsset(asset, metadata);

            return model;
        }

    }

}
