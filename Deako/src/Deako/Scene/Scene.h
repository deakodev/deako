#pragma once

#include "Deako/Asset/Asset.h"

#include "Components.h"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace Deako {

    using SceneRegistry = entt::registry;
    using EntityMap = std::unordered_map<UUID, entt::entity>;

    struct SceneMetadata : public AssetMetadata
    {
        std::string name{ "Untitled" };
    };

    class Entity;

    class Scene : public Asset
    {
    public:
        static Ref<Scene> Open(const std::filesystem::path& path);
        bool Save();

        static void LinkAssets();

        static Ref<Scene> GetActive() { return s_ActiveScene; }

        void OnUpdate();

        void SetName(const std::string& name) { m_Metadata.name = name; }

        const SceneMetadata& GetMetadata() { return m_Metadata; }

        const SceneRegistry& GetRegistry() { return m_Registry; }

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name);
        void DestroyEntity(Entity entity);

        Entity GetEntity(entt::entity handle);
        Entity GetEntity(const std::string& tag);
        Entity GetEntity(UUID uuid);

        template<typename... Components>
        static std::vector<Entity> GetAllEntitiesWith();

        static AssetType GetStaticType() { return AssetType::Scene; }
        virtual AssetType GetType() const override { return GetStaticType(); }

        virtual void Destroy() override {}

    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        SceneMetadata m_Metadata;
        SceneRegistry m_Registry;
        EntityMap m_EntityMap;

        inline static Ref<Scene> s_ActiveScene;

        friend class Entity;
        friend class ScenePanel;
    };

}
