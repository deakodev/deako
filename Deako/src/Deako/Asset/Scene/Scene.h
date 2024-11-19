#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Scene/Components.h"
#include "Deako/Renderer/EditorCamera.h"

#include <entt.hpp>

namespace Deako {

    using EntityHandle = DkHandle;

    class Entity;
    class VulkanScene;

    using SceneRegistry = entt::registry;
    using EnttEntityMap = std::unordered_map<EntityHandle, entt::entity>;
    using PickerColorMap = std::unordered_map<DkVec4, EntityHandle, DkVec4Hash>;

    class Scene : public Asset
    {
    public:
        void Build();
        void Rebuild();
        void CleanUp();

        void OnUpdate();

        bool IsSceneValid() { return m_IsValid && (m_VulkanScene != nullptr); }
        bool IsSceneSaved() { return m_IsSavedUpToDate; }

        static Ref<Scene> Copy(Ref<Scene> srcScene);

        void LinkAssets();

        Entity CreateEntity(const std::string& name = std::string(), EntityHandle handle = EntityHandle());

        void DestroyEntity(Entity entity);

        Entity GetEntity(EntityHandle handle);
        EntityHandle GetEntityHandle(const DkVec4& pickerColor);

        // template<typename... Components>
        // std::vector<Entity> GetAllEntitiesWith();

        void SetActiveCamera(Ref<EditorCamera> camera) { m_ActiveCamera = camera; }

        VulkanScene& GetVulkanScene() { return *m_VulkanScene; }
        std::vector<Entity>& GetEntities() { return m_Entities; }
        EnttEntityMap& GetEnttEntityMap() { return m_EnttEntityMap; }
        SceneRegistry& GetRegistry() { return m_Registry; }
        Ref<EditorCamera> GetActiveCamera() { return m_ActiveCamera; }

        static AssetType GetStaticType() { return AssetType::Scene; }
        virtual AssetType GetType() const override { return GetStaticType(); }

        virtual void Destroy() override {}

        template<typename T>
        void OnComponentAdded(const Entity& entity, T& component);

        virtual void Invalidate() override { m_IsValid = false; }

    private:
        Scope<VulkanScene> m_VulkanScene;

        std::vector<Entity> m_Entities;
        EnttEntityMap m_EnttEntityMap;
        PickerColorMap m_PickerColorMap;
        SceneRegistry m_Registry;

        Ref<EditorCamera> m_ActiveCamera;

        bool m_IsSavedUpToDate{ true };
        bool m_IsValid{ false };

    };

    inline static Ref<Scene> s_EmptyScene;

}
