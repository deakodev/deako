#pragma once

#include "Deako/Asset/Asset.h"

#include "Deako/Asset/Scene/Components.h"
#include "Deako/Renderer/EditorCamera.h"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace Deako {

    using SceneRegistry = entt::registry;
    using EntityMap = std::unordered_map<UUID, entt::entity>;

    class Entity;

    struct Scene : public Asset
    {
        SceneRegistry registry;
        EntityMap entityMap;

        bool isSavedUpToDate{ true };

        static Ref<Scene> Copy(Ref<Scene> other);

        void LinkAssets();

        void OnUpdate(Ref<EditorCamera> camera);

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

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

        // friend class Entity;
        // friend class ScenePanel;
    };

}
