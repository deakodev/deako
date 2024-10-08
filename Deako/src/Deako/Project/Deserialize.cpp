#include "Deserialize.h"
#include "dkpch.h"

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Prefab/Prefab.h"

#include "Serialize.h"

namespace Deako {

    bool Deserialize::Project(Deako::Project& project, const std::filesystem::path& path)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::ParserException e)
        {
            DK_CORE_ERROR("Could not open project from <{0}>\n {1}", path.string(), e.what());
            return false;
        }

        auto projectNode = data["Project"];
        if (!projectNode)
        {
            DK_CORE_ERROR("Could not read project from <{0}>", path.string());
            return false;
        }

        {
            project.name = projectNode["Name"].as<std::string>();

            project.workingDirectory = path.parent_path();
            project.assetDirectory = project.workingDirectory / projectNode["AssetDirectory"].as<std::string>();

            project.projectFilename = path.filename();
            project.assetRegistryFilename = projectNode["AssetRegistryFilename"].as<std::string>();

            project.initialSceneHandle = projectNode["InitialSceneHandle"].as<uint64_t>();
        }

        return true;
    }

    void Deserialize::AssetRegistry(Deako::AssetRegistry& assetRegistry, const std::filesystem::path& path)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::ParserException e)
        {
            DK_CORE_ERROR("Failed to load asset registry file <{0}>\n {1}", path.filename().string(), e.what());
            return;
        }

        auto rootNode = data["AssetRegistry"];
        if (!rootNode)
        {
            DK_CORE_ERROR("Could not read asset registry from <{0}>", path.filename().string());
            return;
        }

        for (const auto& assetNode : rootNode)
        {
            AssetHandle handle = assetNode["AssetHandle"].as<uint64_t>();

            AssetMetadata metadata;
            metadata.assetType = AssetTypeFromString(assetNode["AssetType"].as<std::string>());
            metadata.assetPath = assetNode["AssetPath"].as<std::string>();

            std::string assetName = metadata.assetPath.filename().string();
            assetName[0] = std::toupper(assetName[0]);
            metadata.assetName = assetName;

            assetRegistry[handle] = metadata;
        }
    }

    void Deserialize::Scene(Deako::Scene& scene, AssetMetadata& metadata)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(metadata.assetPath);
        }
        catch (YAML::ParserException e)
        {
            DK_CORE_ERROR("Failed to load scene file <{0}>\n {1}", metadata.assetPath.string(), e.what());
            return;
        }

        auto sceneNode = data["Scene"];
        if (!sceneNode)
        {
            DK_CORE_ERROR("Could not read scene from <{0}>", metadata.assetPath.filename().string());
            return;
        }

        metadata.assetName = sceneNode["Name"].as<std::string>();
        scene.m_Handle = sceneNode["AssetHandle"].as<uint64_t>();

        auto yamlEntities = data["Entities"];
        if (yamlEntities)
        {
            for (auto yamlEntity : yamlEntities)
            {
                uint64_t uuid = yamlEntity["Entity"].as<uint64_t>();

                std::string name;

                auto tagCompYaml = yamlEntity["TagComponent"];
                if (tagCompYaml)
                    name = tagCompYaml["Tag"].as<std::string>();

                Entity entity = scene.CreateEntity(name);

                auto transformCompYaml = yamlEntity["TransformComponent"];
                if (transformCompYaml)
                {
                    // Entities always have transforms so we can use GetComponent<TransformComponent>() freely
                    auto& transformComp = entity.GetComponent<TransformComponent>();
                    transformComp.translation = transformCompYaml["Translation"].as<glm::vec3>();
                    transformComp.rotation = transformCompYaml["Rotation"].as<glm::vec3>();
                    transformComp.scale = transformCompYaml["Scale"].as<glm::vec3>();
                }

                auto textureCompYaml = yamlEntity["TextureComponent"];
                if (textureCompYaml)
                {
                    auto& textureComp = entity.AddComponent<TextureComponent>();
                    textureComp.handle = textureCompYaml["AssetHandle"].as<uint64_t>();
                }

                auto materialCompYaml = yamlEntity["MaterialComponent"];
                if (materialCompYaml)
                {
                    auto& materialComp = entity.AddComponent<MaterialComponent>();
                    materialComp.handle = materialCompYaml["AssetHandle"].as<uint64_t>();
                }

                auto modelCompYaml = yamlEntity["ModelComponent"];
                if (modelCompYaml)
                {
                    auto& modelComp = entity.AddComponent<ModelComponent>();
                    modelComp.handle = modelCompYaml["AssetHandle"].as<uint64_t>();
                }

                auto prefabCompYaml = yamlEntity["PrefabComponent"];
                if (prefabCompYaml)
                {
                    auto& prefabComp = entity.AddComponent<PrefabComponent>();
                    prefabComp.handle = prefabCompYaml["AssetHandle"].as<uint64_t>();

                    Ref<Prefab> prefab = ProjectAssetPool::Get()->GetAsset<Prefab>(prefabComp.handle);

                    prefabComp.meshHandle = prefab->model->m_Handle;

                    for (const auto& [textureHandle, texture] : prefab->textures)
                        prefabComp.textureHandles.emplace_back(textureHandle);

                    for (const auto& [materialHandle, material] : prefab->materials)
                        prefabComp.materialHandles.emplace_back(materialHandle);
                }

                auto envCompYaml = yamlEntity["EnvironmentComponent"];
                if (envCompYaml)
                {
                    auto& envComp = entity.AddComponent<EnvironmentComponent>();
                    envComp.active = envCompYaml["Active"].as<bool>();
                }

                DK_CORE_INFO("Deser Entity '{0}' [{1}]", name, uuid);
            }
        }
    }


}
