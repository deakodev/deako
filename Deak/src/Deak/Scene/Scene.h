#pragma once

#include "Deak/Renderer/Camera/EditorCameraController.h"
#include "Deak/Core/Timestep.h"
#include "entt.hpp"

namespace Deak {

    class Entity;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void OnUpdateEditor(Timestep timestep, EditorCameraController& cameraController);
        void OnUpdateRuntime(Timestep timestep);
        void OnViewportResize(float width, float height);

        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        Entity GetPrimaryCameraEntity();

    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        entt::registry m_Registry;

        float m_ViewportWidth = 0.0f;
        float m_ViewportHeight = 0.0f;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

}
