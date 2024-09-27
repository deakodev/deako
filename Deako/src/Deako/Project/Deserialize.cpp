#include "Deserialize.h"
#include "dkpch.h"

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Prefab.h"

#include "Serialize.h"

namespace Deako {

    Ref<Project> Deserialize::Project(const std::filesystem::path& path)
    {
        Ref<Deako::Project> project = CreateRef<Deako::Project>();

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::ParserException e)
        {
            DK_CORE_ERROR("Failed to load project file <{0}>\n {1}", path.string(), e.what());
            return nullptr;
        }

        auto projectNode = data["Project"];
        if (!projectNode) return nullptr;

        project->SetMetadata({
            projectNode["Name"].as<std::string>(),
            path.parent_path(),
            projectNode["AssetDirectory"].as<std::string>(),
            (projectNode["AssetRegistryPath"]) ? projectNode["AssetRegistryPath"].as<std::string>() : "",
            (projectNode["InitialScenePath"]) ? projectNode["InitialScenePath"].as<std::string>() : "",
            });

        DK_CORE_INFO("Deser Project '{0}'", projectNode["Name"].as<std::string>());

        return project;
    }

    Ref<AssetRegistry> Deserialize::AssetRegistry(const std::filesystem::path& path)
    {
        Ref<Deako::AssetRegistry> assetRegistry = CreateRef<Deako::AssetRegistry>();

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::ParserException e)
        {
            DK_CORE_ERROR("Failed to load asset registry file <{0}>\n {1}", path.string(), e.what());
            return nullptr;
        }

        auto rootNode = data["AssetRegistry"];
        if (!rootNode) return assetRegistry;

        for (const auto& assetNode : rootNode)
        {
            AssetHandle handle = assetNode["AssetHandle"].as<uint64_t>();

            AssetMetadata metadata;
            metadata.assetType = AssetTypeFromString(assetNode["AssetType"].as<std::string>());
            metadata.assetPath = assetNode["AssetPath"].as<std::string>();

            (*assetRegistry)[handle] = metadata;
        }

        DK_CORE_INFO("Deser AssetRegistry");

        return assetRegistry;
    }

    Ref<Scene> Deserialize::Scene(const std::filesystem::path& path)
    {
        Ref<Deako::Scene> scene = CreateRef<Deako::Scene>();

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path);
        }
        catch (YAML::ParserException e)
        {
            DK_CORE_ERROR("Failed to load scene file <{0}>\n {1}", path.string(), e.what());
            return nullptr;
        }

        auto sceneNode = data["Scene"];
        if (!sceneNode) return nullptr;

        scene->SetName(sceneNode["Name"].as<std::string>());

        DK_CORE_INFO("Deser Scene '{0}' <{1}>",
            sceneNode["Name"].as<std::string>(), path.filename().string());

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

                Entity entity = scene->CreateEntity(name);

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
                    auto textureHandlesNode = textureCompYaml["AssetHandles"];

                    if (textureHandlesNode && textureHandlesNode.IsSequence())
                    {
                        for (const auto& handleNode : textureHandlesNode)
                        {
                            textureComp.handles.push_back(handleNode.as<uint64_t>());
                        }
                    }
                }

                auto materialCompYaml = yamlEntity["MaterialComponent"];
                if (materialCompYaml)
                {
                    auto& materialComp = entity.AddComponent<MaterialComponent>();
                    auto materialHandlesNode = materialCompYaml["AssetHandles"];

                    if (materialHandlesNode && materialHandlesNode.IsSequence())
                    {
                        for (const auto& handleNode : materialHandlesNode)
                        {
                            materialComp.handles.push_back(handleNode.as<uint64_t>());
                        }
                    }
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

        return scene;
    }


}
