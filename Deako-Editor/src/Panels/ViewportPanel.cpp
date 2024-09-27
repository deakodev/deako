#include "ViewportPanel.h"

#include "../EditorLayer.h"

namespace Deako {

    void ViewportPanel::OnUpdate()
    {
        if (m_ViewportResize)
        {
            VulkanScene::ViewportResize(m_ViewportSize); // TODO: abstract from vulkan when working on camera
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
                const char* path = (const char*)payload->Data;
                DK_TRACE("Payload: {0}", path);
                EditorLayer::OpenScene(path);
                Renderer::Invalidate();
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

}
