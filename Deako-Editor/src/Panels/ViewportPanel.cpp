#include "ViewportPanel.h"

namespace Deako {

    void ViewportPanel::OnUpdate()
    {
        Scene& activeScene = Deako::GetActiveScene();

        if (m_ViewportResize)
        {
            activeScene.activeCamera->ResizeCamera(m_ViewportSize);
            m_ViewportResize = false;
        }
    }

    void ViewportPanel::OnImGuiRender()
    {
        DkContext& deako = Deako::GetContext();
        Scene& activeScene = Deako::GetActiveScene();
        ProjectAssetPool& projectAssetPool = Deako::GetProjectAssetPool();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        // m_ViewportFocused = ImGui::IsWindowFocused();
        // m_ViewportHovered = ImGui::IsWindowHovered();
        // ImGuiLayer::BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        if (m_ViewportSize.x != viewport->Size.x || m_ViewportSize.y != viewport->Size.y)
            m_ViewportResize = true;

        m_ViewportOrigin = { viewport->Pos.x, viewport->Pos.y };
        m_ViewportSize = { viewport->Size.x, viewport->Size.y };

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
            {
                AssetHandle assetHandle = *(AssetHandle*)payload->Data;
                AssetType assetType = projectAssetPool.GetAssetType(assetHandle);

                if (assetType == AssetType::Scene)
                {
                    Deako::SetActiveScene(assetHandle);
                    SceneHandler::RefreshScene();
                }
                else if (assetType == AssetType::Prefab)
                {
                    Entity entity = activeScene.CreateEntity("New Prefab");
                    entity.AddComponent<PrefabComponent>(assetHandle);
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
        if ((m_GizmoOperation != -1) && (deako.activeHandle != 0))
            TransformWithGizmo(deako.activeHandle);
    }

    void ViewportPanel::TransformWithGizmo(EntityHandle handle)
    {
        Scene& activeScene = Deako::GetActiveScene();
        Ref<EditorCamera> camera = activeScene.activeCamera;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        m_ViewportOrigin.y += m_ViewportSize.y; // flip origin to bottom
        ImGuizmo::SetRect(m_ViewportOrigin.x, m_ViewportOrigin.y, m_ViewportSize.x, -m_ViewportSize.y);

        DkMat4 cameraView = camera->GetView();
        DkMat4 cameraProjection = camera->GetProjection();

        DkMat4 flipY = glm::scale(DkMat4(1.0f), DkVec3(1.0f, -1.0f, 1.0f));
        cameraProjection = cameraProjection * flipY;

        // selected entity's transform
        auto& transformComp = Entity::GetComponent<TransformComponent>(handle);
        DkMat4 modelMatrix = transformComp.GetTransform();
        modelMatrix = flipY * modelMatrix;

        bool snap = Deako::IsKeyPressed(Key::LeftSuper);
        DkF32 snapValue = m_GizmoOperation == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f; // 45deg rot, 0.5m tranlation/scale
        DkF32 snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
            (ImGuizmo::OPERATION)m_GizmoOperation, ImGuizmo::LOCAL, glm::value_ptr(modelMatrix), nullptr, (snap ? snapValues : nullptr));

        if (ImGuizmo::IsUsing())
        {
            modelMatrix = flipY * modelMatrix;

            DkVec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

            if (m_GizmoOperation == ImGuizmo::OPERATION::TRANSLATE) {
                transformComp.translation = translation;
            }
            else if (m_GizmoOperation == ImGuizmo::OPERATION::ROTATE) {
                DkVec3 newRotation = transformComp.rotation - glm::radians(rotation);

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
