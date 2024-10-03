#pragma once

#include "Deako/Asset/Asset.h"

#include "Components.h"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace Deako {

    using SceneRegistry = entt::registry;
    using EntityMap = std::unordered_map<UUID, entt::entity>;

    class Entity;

    class Scene : public Asset
    {
    public:
        void LinkAssets();

        const SceneRegistry& GetRegistry() { return m_Registry; }

        void OnUpdate();

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name);
        void DestroyEntity(Entity entity);

        Entity GetEntity(entt::entity handle);
        Entity GetEntity(const std::string& tag);
        Entity GetEntity(UUID uuid);

        template<typename... Components>
        std::vector<Entity> GetAllEntitiesWith();

        static AssetType GetStaticType() { return AssetType::Scene; }
        virtual AssetType GetType() const override { return GetStaticType(); }

        virtual void Destroy() override {}

    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        SceneRegistry m_Registry;
        EntityMap m_EntityMap;

        friend class Entity;
        friend class ScenePanel;
    };

}
