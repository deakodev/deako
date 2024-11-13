#pragma once

#include "Deako/Asset/Asset.h"

#include "Deako/Asset/Scene/Components.h"
#include "Deako/Renderer/EditorCamera.h"

#include <entt.hpp>


namespace Deako {

    using EntityHandle = DkHandle;

    class Entity;

    using SceneRegistry = entt::registry;
    using EntityMap = std::unordered_map<EntityHandle, entt::entity>;
    using PickerColorMap = std::unordered_map<DkVec4, EntityHandle, DkVec4Hash>;

    struct Scene : public Asset
    {
        SceneRegistry registry;
        std::vector<Entity> entities;
        EntityMap entityMap;
        PickerColorMap pickerColorMap;
        Ref<EditorCamera> activeCamera;

        bool isSavedUpToDate{ true };
        bool isValid{ false };

        static Ref<Scene> Copy(Ref<Scene> srcScene);

        void LinkAssets();

        void OnUpdate();

        Entity CreateEntity(const std::string& name = std::string(), EntityHandle handle = EntityHandle());

        void DestroyEntity(Entity entity);

        Entity GetEntity(EntityHandle handle);
        EntityHandle GetEntityHandle(const DkVec4& pickerColor);

        // template<typename... Components>
        // std::vector<Entity> GetAllEntitiesWith();

        EntityMap& GetEntityMap() { return entityMap; }

        static AssetType GetStaticType() { return AssetType::Scene; }
        virtual AssetType GetType() const override { return GetStaticType(); }

        virtual void Destroy() override {}

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    };

    inline static Ref<Scene> s_EmptyScene;

}
