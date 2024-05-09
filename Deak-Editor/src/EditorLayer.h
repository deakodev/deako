#pragma once

#include "Panels/SceneHierarchyPanel.h"
#include "Deak/Renderer/Camera/EditorCameraController.h"

namespace Deak {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Timestep timestep) override;
        virtual void OnEvent(Event& event) override;

        virtual void OnImGuiRender(Timestep timestep) override;

    private:
        bool OnKeyPressed(KeyPressedEvent& event);
        bool OnEditorClose(WindowCloseEvent& event);

        void NewScene();
        void OpenScene();
        void SaveScene();
        void SaveSceneAs();

        void Close();

    private:
        EditorCameraController  m_CameraController;
        Ref<Texture2D> m_BoxTexture;
        Ref<Texture2D> m_HealthBarTexture;
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;
        Entity m_BoxEntity;
        Entity m_FloorEntity;
        Entity m_WallEntity;
        Entity m_HealthBarEntity;
        Entity m_HUDCameraEntity;
        Entity m_PrimaryCameraEntity;

        glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

        bool m_PrimaryCamera = true;

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

        int m_GizmoType = -1;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;

    };

}
