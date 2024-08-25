#include "Serialize.h"
#include "dkpch.h"

#include "Deako/Asset/Asset.h"

#include <fstream>

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

    bool Serialize::Project()
    {
        const std::filesystem::path& path = Project::GetProjectDirectory();
        const ProjectDetails& details = Project::GetActive()->GetDetails();

        YAML::Emitter out;
        {
            out << YAML::BeginMap; // Root
            out << YAML::Key << "Project" << YAML::Value;
            {
                out << YAML::BeginMap; // Project
                out << YAML::Key << "Name" << YAML::Value << details.name;
                out << YAML::Key << "AssetDirectory" << YAML::Value << details.assetDirectory.string();
                out << YAML::Key << "AssetRegistryPath" << YAML::Value << details.assetRegistryPath.string();
                out << YAML::EndMap; // Project
            }
            out << YAML::EndMap; // Root
        }

        std::ofstream fout(path.string());
        fout << out.c_str();

        return true;
    }

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

        project->SetDetails({
            projectNode["Name"].as<std::string>(),
            projectNode["AssetDirectory"].as<std::string>(),
            (projectNode["AssetRegistryPath"]) ? projectNode["AssetRegistryPath"].as<std::string>() : "",
            });

        DK_CORE_TRACE("Deserialized Project '{0}'", projectNode["Name"].as<std::string>());

        return project;
    }

    bool Serialize::AssetRegistry()
    {
        const std::filesystem::path& path = Project::GetAssetRegistryPath();
        const Deako::AssetRegistry& assetRegistry = Project::GetActive()->GetEditorAssetPool()->GetAssetRegistry();

        YAML::Emitter out;
        {
            out << YAML::BeginMap; // Root
            out << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

            for (const auto& [handle, metadata] : assetRegistry)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << handle;
                out << YAML::Key << "Path" << YAML::Value << metadata.path.string();
                out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.type);
                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap; // Root
        }

        std::ofstream fout(path.string());
        fout << out.c_str();

        return true;
    }

    Ref<AssetRegistry> Deserialize::AssetRegistry()
    {
        const std::filesystem::path& path = Project::GetAssetRegistryPath();
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

        for (const auto& node : rootNode)
        {
            AssetHandle handle = node["Handle"].as<uint64_t>();

            AssetMetadata& metadata = (*assetRegistry)[handle];
            metadata.path = node["Path"].as<std::string>();
            metadata.type = AssetTypeFromString(node["Type"].as<std::string>());
        }

        DK_CORE_TRACE("Deserialized AssetRegistry");

        return assetRegistry;
    }

    bool Serialize::Scene(Deako::Scene& scene)
    {
        const SceneDetails& details = scene.GetDetails();
        const entt::registry& registry = scene.GetRegistry();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << details.name;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        for (auto handle : registry.view<entt::entity>())
        {
            Deako::Entity entity{ handle, &scene };
            if (!entity) return false;

            Serialize::Entity(out, entity);
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(details.path);
        fout << out.c_str();

        return true;
    }

    Ref<Scene> Deserialize::Scene(const std::filesystem::path& path)
    {
        Ref<Deako::Scene> scene = CreateRef<Deako::Scene>(path);

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

        scene->SetDetails({
            sceneNode["Name"].as<std::string>(),
            sceneNode["Filename"].as<std::string>()
            });

        DK_CORE_TRACE("Deserialized Scene '{0}'", sceneNode["Name"].as<std::string>());

        auto yamlEntities = data["Entities"];
        if (yamlEntities)
        {
            for (auto yamlEntity : yamlEntities)
            {
                uint64_t uuid = yamlEntity["Entity"].as<uint64_t>(); // TODO: uuid

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

                auto modelCompYaml = yamlEntity["ModelComponent"];
                if (modelCompYaml)
                {
                    auto& modelComp = entity.AddComponent<ModelComponent>();
                    // modelComp.model->path = modelCompYaml["Path"].as<std::string>();
                    modelComp.usage = static_cast<ModelComponent::Usage>(modelCompYaml["Usage"].as<uint32_t>());
                }

                auto textureCompYaml = yamlEntity["TextureComponent"];
                if (textureCompYaml)
                {
                    auto& textureComp = entity.AddComponent<TextureComponent>();
                    textureComp.texture = textureCompYaml["TextureHandle"].as<uint64_t>();
                }

                DK_CORE_TRACE("Deserializied Entity '{0}' [{1}]", name, uuid);
            }
        }

        return scene;
    }

    void Serialize::Entity(YAML::Emitter& out, Deako::Entity entity)
    {
        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << "1234567890"; // TODO: uuid 

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

        if (entity.HasComponent<ModelComponent>())
        {
            out << YAML::Key << "ModelComponent";
            out << YAML::BeginMap; // ModelComponent

            auto& modelComp = entity.GetComponent<ModelComponent>();
            // out << YAML::Key << "Path" << YAML::Value << modelComp.model->path;
            out << YAML::Key << "Usage" << YAML::Value << modelComp.usage;

            out << YAML::EndMap; // ModelComponent
        }

        if (entity.HasComponent<TextureComponent>())
        {
            out << YAML::Key << "TextureComponent";
            out << YAML::BeginMap; // TextureComponent

            auto& textureComp = entity.GetComponent<TextureComponent>();
            out << YAML::Key << "TextureHandle" << YAML::Value << textureComp.texture;

            out << YAML::EndMap; // TextureComponent
        }

        out << YAML::EndMap; // Entity
    }

} // End Deako namespace

namespace YAML {

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

}
