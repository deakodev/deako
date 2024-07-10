#include "EditorLayer.h"

#include <imgui/imgui.h>

namespace Deako {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate()
    {
        VulkanBase::DrawFrame();
    }

    void EditorLayer::OnImGuiRender()
    {
        ImGui::ShowDemoWindow();

        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& event)
    {
    }

}
