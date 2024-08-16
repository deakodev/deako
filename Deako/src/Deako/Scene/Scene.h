#pragma once

#include "Deako/Renderer/EditorCamera.h"

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

    class Scene
    {
    public:
        Scene(const std::filesystem::path& path);

        using Registry = entt::registry;

        static Ref<Scene> Open(const std::string& filename);
        bool Save();

        void Prepare();

        void OnUpdateEditor(Camera& editorCamera);

        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        static Ref<Scene> GetActiveScene() { return s_ActiveScene; }
        std::unordered_map<std::string, Ref<Model>> GetModels();

        void SetDetails(SceneDetails details) { m_Details = details; }
        const SceneDetails& GetDetails() { return m_Details; }

        const Registry& GetRegistry() { return m_Registry; }

    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        Registry m_Registry;
        SceneDetails m_Details;

        float m_ViewportWidth = 0.0f;
        float m_ViewportHeight = 0.0f;

        friend class Entity;
        // friend class SceneHierarchyPanel;

        inline static Ref<Scene> s_ActiveScene;
    };

}
