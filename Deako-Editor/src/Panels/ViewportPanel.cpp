#include "ViewportPanel.h"

#include "../EditorLayer.h"
#include "Deako/Math/Math.h"

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
        if (m_GizmoOperation != -1 && m_EditorContext->entity)
            TransformWithGizmo();


        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewportPanel::TransformWithGizmo()
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        auto [viewportOriginX, viewportOriginY] = ImGui::GetWindowPos();
        viewportOriginY += m_ViewportSize.y; // flip origin to bottom
        ImGuizmo::SetRect(viewportOriginX, viewportOriginY, m_ViewportSize.x, -m_ViewportSize.y);

        glm::mat4 cameraView = m_EditorCamera->GetView();
        glm::mat4 cameraProjection = m_EditorCamera->GetProjection();

        glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
        cameraProjection = cameraProjection * flipY;

        // selected entity's transform
        auto& transformComp = m_EditorContext->entity->GetComponent<TransformComponent>();
        glm::mat4 modelMatrix = transformComp.GetTransform();
        modelMatrix = flipY * modelMatrix;

        bool snap = Input::IsKeyPressed(Key::LeftSuper);
        float snapValue = m_GizmoOperation == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f; // 45deg rot, 0.5m tranlation/scale
        float snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
            (ImGuizmo::OPERATION)m_GizmoOperation, ImGuizmo::LOCAL, glm::value_ptr(modelMatrix), nullptr, (snap ? snapValues : nullptr));

        if (ImGuizmo::IsUsing())
        {
            modelMatrix = flipY * modelMatrix;

            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

            if (m_GizmoOperation == ImGuizmo::OPERATION::TRANSLATE) {
                transformComp.translation = translation;
            }
            else if (m_GizmoOperation == ImGuizmo::OPERATION::ROTATE) {
                glm::vec3 newRotation = transformComp.rotation - glm::radians(rotation);

                transformComp.rotation += newRotation;
                // Normalize the rotation after applying new rotation
                transformComp.rotation = transformComp.NormalizeRotation(transformComp.rotation);
            }
            else if (m_GizmoOperation == ImGuizmo::OPERATION::SCALE) {
                transformComp.scale = scale;
            }
        }
    }

}
