#include "ViewportPanel.h"

#include "../EditorLayer.h"

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

        ImGui::End();
        ImGui::PopStyleVar();
    }

}
