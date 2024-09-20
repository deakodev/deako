#pragma once

#include "Deako/Renderer/EditorCamera.h"
#include "Deako/Asset/Asset.h"

#include "Components.h"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace Deako {

    class Entity;

    struct SceneDetails
    {
        std::string name{ "Untitled" };
        std::filesystem::path path;
    };

    class Scene : public Asset
    {
    public:
        Scene(const std::filesystem::path& path);

        using Registry = entt::registry;

        static Ref<Scene> Open(const std::filesystem::path& path);
        bool Save();

        void OnUpdateEditor(Camera& editorCamera);

        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        static void SetActive(Ref<Scene> scene) { s_ActiveScene = scene; }
        static Ref<Scene> GetActive() { return s_ActiveScene; }

        void SetDetails(SceneDetails details) { m_Details = details; }
        const SceneDetails& GetDetails() { return m_Details; }

        const Registry& GetRegistry() { return m_Registry; }

        Entity GetEntityByName(const std::string& name);
        // Entity GetEntityByUUID(EntityID id);

        template<typename... Components>
        auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        static AssetType GetStaticType() { return AssetType::Scene; }
        virtual AssetType GetType() const override { return GetStaticType(); }

        virtual void Destroy() override {}

    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        Registry m_Registry;
        SceneDetails m_Details;

        friend class Entity;
        friend class SceneHierarchyPanel;

        inline static Ref<Scene> s_ActiveScene;
    };

}
