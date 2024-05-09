#include "SceneSerializer.h"
#include "dkpch.h"

#include "Entity.h"
#include "Components.h"

#include <yaml-cpp/yaml.h>

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

namespace Deak {

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

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : m_Scene(scene)
    {
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << "1234567890"; // TODO: entity uuid 

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

        if (entity.HasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap; // CameraComponent

            auto& cameraComp = entity.GetComponent<CameraComponent>();
            auto& camera = cameraComp.camera;

            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap; // Camera
            out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFov();
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNear();
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFar();
            out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
            out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNear();
            out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFar();
            out << YAML::EndMap; // Camera

            out << YAML::Key << "Primary" << YAML::Value << cameraComp.primary;
            out << YAML::Key << "HUD" << YAML::Value << cameraComp.hud;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComp.fixedAspectRatio;

            out << YAML::EndMap; // CameraComponent
        }

        if (entity.HasComponent<SpriteRendererComponent>())
        {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap; // SpriteRendererComponent

            auto& spriteRendererComp = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComp.color;

            out << YAML::EndMap; // SpriteRendererComponent
        }

        if (entity.HasComponent<OverlayComponent>())
        {
            out << YAML::Key << "OverlayComponent";
            out << YAML::BeginMap; // OverlayComponent

            auto& overlayComp = entity.GetComponent<OverlayComponent>();
            out << YAML::Key << "Color" << YAML::Value << overlayComp.color;

            out << YAML::EndMap; // OverlayComponent
        }

        out << YAML::EndMap; // Entity
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        auto view = m_Scene->m_Registry.view<TagComponent>();
        for (auto _entity : view)
        {
            Entity entity{ _entity, m_Scene.get() };
            if (!entity)
                return;

            SerializeEntity(out, entity);
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout{ filepath };
        fout << out.c_str();
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath)
    {
        // Not implemented
        DK_CORE_ASSERT(false, "");
    }

    bool SceneSerializer::Deserialize(const std::string& filepath)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath);
        }
        catch (YAML::ParserException e)
        {
            DK_CORE_ERROR("Failed to load .deak file '{0}'\n     {1}", filepath, e.what());
            return false;
        }

        if (!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();
        DK_CORE_TRACE("Deserializing scene '{0}'", sceneName);

        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>(); //TODO: uuid

                std::string name;
                auto tagCompYaml = entity["TagComponent"];
                if (tagCompYaml)
                    name = tagCompYaml["Tag"].as<std::string>();

                DK_CORE_TRACE("Deserializied entity with ID = {0}, name = {1}", uuid, name);

                Entity deserializedEntity = m_Scene->CreateEntity(name);

                auto transformCompYaml = entity["TransformComponent"];
                if (transformCompYaml)
                {
                    // Entities always have transforms so we can use GetComponent<TransformComponent>() freely
                    auto& transformComp = deserializedEntity.GetComponent<TransformComponent>();
                    transformComp.translation = transformCompYaml["Translation"].as<glm::vec3>();
                    transformComp.rotation = transformCompYaml["Rotation"].as<glm::vec3>();
                    transformComp.scale = transformCompYaml["Scale"].as<glm::vec3>();
                }

                auto cameraCompYaml = entity["CameraComponent"];
                if (cameraCompYaml)
                {
                    auto& cameraComp = deserializedEntity.AddComponent<CameraComponent>();
                    auto& camera = cameraComp.camera;

                    const auto& cameraProps = cameraCompYaml["Camera"];
                    camera.SetProjectionType((ProjectionType)cameraProps["ProjectionType"].as<int>());
                    camera.SetPerspectiveVerticalFov(cameraProps["PerspectiveFOV"].as<float>());
                    camera.SetPerspectiveNear(cameraProps["PerspectiveNear"].as<float>());
                    camera.SetPerspectiveFar(cameraProps["PerspectiveFar"].as<float>());
                    camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                    camera.SetOrthographicNear(cameraProps["OrthographicNear"].as<float>());
                    camera.SetOrthographicFar(cameraProps["OrthographicFar"].as<float>());
                    cameraComp.primary = cameraCompYaml["Primary"].as<bool>();
                    cameraComp.hud = cameraCompYaml["HUD"].as<bool>();
                    cameraComp.fixedAspectRatio = cameraCompYaml["FixedAspectRatio"].as<bool>();
                }

                auto spriteRendererCompYaml = entity["SpriteRendererComponent"];
                if (spriteRendererCompYaml)
                {
                    auto& spriteRendererComp = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    spriteRendererComp.color = spriteRendererCompYaml["Color"].as<glm::vec4>();
                }

            }
        }

        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
    {
        // Not implemented
        DK_CORE_ASSERT(false, "");
        return false;
    }

}
