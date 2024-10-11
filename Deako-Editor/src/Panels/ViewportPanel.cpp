#include "ViewportPanel.h"

#include "../EditorLayer.h"
#include "Deako/Math/Math.h"

#include <ImGuizmo.h>

namespace Deako {

    ViewportPanel::ViewportPanel(Ref<EditorContext> editorContext, Ref<EditorCamera> editorCamera)
        : m_EditorContext(editorContext), m_EditorCamera(editorCamera)
    {
    }

    void ViewportPanel::OnUpdate()
    {
        if (m_ViewportResize)
        {
            m_EditorCamera->ResizeCamera(m_ViewportSize);
            m_ViewportResize = false;
        }
    }

    void ViewportPanel::OnImGuiRender(ImTextureID textureID)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
        ImGui::Begin("Viewport");

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        ImGuiLayer::BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y)
            m_ViewportResize = true;

        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 0), ImVec2(1, 1));
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
            {
                AssetHandle handle = *(AssetHandle*)payload->Data;
                AssetType assetType = m_EditorContext->assetPool->GetAssetType(handle);

                if (assetType == AssetType::Scene)
                {
                    SceneHandler::SetActiveScene(handle);
                    SceneHandler::RefreshScene();
                    m_EditorContext->scene.isValid = false;
                }
                else if (assetType == AssetType::Prefab)
                {
                    Entity entity = m_EditorContext->scene->CreateEntity("New Prefab");
                    entity.AddComponent<PrefabComponent>(handle);
                    SceneHandler::RefreshScene();
                }
                else
                {
                    DK_WARN("Wrong asset type!");
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Gizmos
        if (m_EditorContext->entity)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            // get camera matrices
            glm::mat4 cameraView = m_EditorCamera->GetView();
            glm::mat4 cameraProjection = m_EditorCamera->GetProjection();

            glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
            cameraProjection = cameraProjection * flipY;

            // get the selected entity transform
            auto& transformComp = m_EditorContext->entity->GetComponent<TransformComponent>();
            glm::mat4 selectedEntityTransform = transformComp.GetTransform();

            // Apply the Y-axis flip to the selected entity's transform
            selectedEntityTransform = flipY * selectedEntityTransform;

            bool snap = Input::IsKeyPressed(Key::LeftSuper);
            float snapValue = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f; // 45deg rot, 0.5m tranlation/scale
            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::WORLD, glm::value_ptr(selectedEntityTransform),
                nullptr, (snap ? snapValues : nullptr));

            // selectedEntityTransform = flipY * selectedEntityTransform;

            static glm::mat4 origin = 1.0f;
            ImGuizmo::DrawGrid(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), glm::value_ptr(origin), 10.0);

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, rotation, scale;
                Math::DecomposeTransform(selectedEntityTransform, translation, rotation, scale);

                translation.y = -translation.y;
                transformComp.translation = translation;

                glm::vec3 deltaRotation = rotation - transformComp.rotation;
                transformComp.rotation += deltaRotation;

                transformComp.scale = scale;
            }
        }


        ImGui::End();
        ImGui::PopStyleVar();
    }

}
