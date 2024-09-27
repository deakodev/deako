#include "Serialize.h"
#include "dkpch.h"

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Prefab.h"

namespace Deako {

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    bool Serialize::Project(Deako::Project& project)
    {
        const ProjectMetadata& metadata = project.GetMetadata();

        YAML::Emitter out;
        {
            out << YAML::BeginMap; // Root
            out << YAML::Key << "Project" << YAML::Value;
            {
                out << YAML::BeginMap; // Project
                out << YAML::Key << "Name" << YAML::Value << metadata.name;
                out << YAML::Key << "WorkingDirectory" << YAML::Value << metadata.workingDirectory.string();
                out << YAML::Key << "AssetDirectory" << YAML::Value << metadata.assetDirectory.string();
                out << YAML::Key << "AssetRegistryPath" << YAML::Value << metadata.assetRegistryPath.string();
                out << YAML::Key << "InitialScenePath" << YAML::Value << metadata.initialScenePath.string();
                out << YAML::EndMap; // Project
            }
            out << YAML::EndMap; // Root
        }

        std::ofstream fout(metadata.workingDirectory.string());
        fout << out.c_str();

        return true;
    }

    bool Serialize::AssetRegistry()
    {
        const std::filesystem::path& path = Project::GetActive()->GetAssetRegistryPath();
        const Deako::AssetRegistry& assetRegistry = Project::GetActive()->GetAssetPool()->GetAssetRegistry();

        YAML::Emitter out;
        {
            out << YAML::BeginMap; // Root
            out << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

            for (const auto& [handle, metadata] : assetRegistry)
            {
                if (metadata.assetPath.empty()) continue; // TODO: maybe get the dependent assets metadata

                out << YAML::BeginMap;
                out << YAML::Key << "AssetHandle" << YAML::Value << handle;
                out << YAML::Key << "AssetPath" << YAML::Value << metadata.assetPath.filename().string();
                out << YAML::Key << "AssetType" << YAML::Value << AssetTypeToString(metadata.assetType);
                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap; // Root
        }

        std::ofstream fout(path.string());
        fout << out.c_str();

        return true;
    }

    bool Serialize::Scene(Deako::Scene& scene)
    {
        const SceneMetadata& metadata = scene.GetMetadata();
        const entt::registry& registry = scene.GetRegistry();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << metadata.name;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        for (auto entityHandle : registry.view<entt::entity>())
        {
            Deako::Entity entity{ entityHandle, &scene };
            if (!entity) return false;

            Serialize::Entity(out, entity);
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(metadata.assetPath);
        fout << out.c_str();

        return true;
    }

    void Serialize::Entity(YAML::Emitter& out, Deako::Entity entity)
    {
        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

        if (entity.HasComponent<TagComponent>())
        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            auto& tag = entity.GetComponent<TagComponent>().tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap; // TagComponent
        }

        if (entity.HasComponent<TransformComponent>())
        {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent

            auto& transformComp = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << transformComp.translation;
            out << YAML::Key << "Rotation" << YAML::Value << transformComp.rotation;
            out << YAML::Key << "Scale" << YAML::Value << transformComp.scale;

            out << YAML::EndMap; // TransformComponent
        }

        if (entity.HasComponent<TextureComponent>())
        {
            out << YAML::Key << "TextureComponent";
            out << YAML::BeginMap; // TextureComponent

            auto& textureComp = entity.GetComponent<TextureComponent>();

            out << YAML::Key << "AssetHandles" << YAML::Value << YAML::BeginSeq;

            for (auto handle : textureComp.handles)
            {
                out << handle;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap; // TextureComponent
        }


        if (entity.HasComponent<MaterialComponent>())
        {
            out << YAML::Key << "MaterialComponent";
            out << YAML::BeginMap; // MaterialComponent

            auto& materialComp = entity.GetComponent<MaterialComponent>();

            out << YAML::Key << "AssetHandles" << YAML::Value << YAML::BeginSeq;

            for (auto handle : materialComp.handles)
            {
                out << handle;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap; // MaterialComponent
        }

        if (entity.HasComponent<ModelComponent>())
        {
            out << YAML::Key << "ModelComponent";
            out << YAML::BeginMap; // ModelComponent

            auto& modelComp = entity.GetComponent<ModelComponent>();
            out << YAML::Key << "AssetHandle" << YAML::Value << modelComp.handle;

            out << YAML::EndMap; // ModelComponent
        }

        if (entity.HasComponent<PrefabComponent>())
        {
            out << YAML::Key << "PrefabComponent";
            out << YAML::BeginMap; // PrefabComponent

            auto& prefabComp = entity.GetComponent<PrefabComponent>();
            out << YAML::Key << "AssetHandle" << YAML::Value << prefabComp.handle;

            out << YAML::EndMap; // PrefabComponent
        }

        if (entity.HasComponent<EnvironmentComponent>())
        {
            out << YAML::Key << "EnvironmentComponent";
            out << YAML::BeginMap; // EnvironmentComponent

            auto& envComp = entity.GetComponent<EnvironmentComponent>();
            out << YAML::Key << "Active" << YAML::Value << envComp.active;

            out << YAML::EndMap; // EnvironmentComponent
        }

        out << YAML::EndMap; // Entity
    }

} // End Deako namespace
