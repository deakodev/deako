#include "ImGuiLayer.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <GLFW/glfw3.h>

namespace Deako {

    // Ref<VulkanResources> ImGuiLayer::s_VR;
    // Ref<VulkanSettings> ImGuiLayer::s_VS;

    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    void ImGuiLayer::OnAttach()
    {
        // ImGuiIO& io = ImGui::GetIO(); (void)io;

        // io.Fonts->AddFontFromFileTTF("Deako-Editor/assets/fonts/roboto/Roboto-Bold.ttf", 16.0f);
        // io.FontDefault = io.Fonts->AddFontFromFileTTF("Deako-Editor/assets/fonts/roboto/Roboto-Regular.ttf", 16.0f);

        // ImGuiStyle& style = ImGui::GetStyle();
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     style.WindowRounding = 0.0f;
        //     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        // }

        // style.WindowMenuButtonPosition = ImGuiDir_None;
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        // ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
        // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));

        // SetDarkThemeColors();
    }

    void ImGuiLayer::OnDetach()
    {
    }

    void ImGuiLayer::OnEvent(Event& event)
    {
        // if (m_BlockEvents)
        // {
        //     ImGuiIO& io = ImGui::GetIO();
        //     event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        //     event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        // }
    }

    void ImGuiLayer::Begin()
    {
        // ImGui_ImplVulkan_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();
    }

    void ImGuiLayer::End(VkCommandBuffer commandBuffer)
    {
        // ImGui::Render();
        // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

        // ImGuiIO& io = ImGui::GetIO();
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backupCurrentContext);
        // }
    }

    void ImGuiLayer::SetDarkThemeColors()
    {
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1216f, 0.1373f, 0.1608f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.1569f, 0.1725f, 0.2039f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.1669f, 0.1825f, 0.2139f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.1669f, 0.1825f, 0.2139f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.1569f, 0.1725f, 0.2039f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.1669f, 0.1825f, 0.2139f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.0835f, 0.0913f, 0.1070f, 1.0f };

        colors[ImGuiCol_CheckMark] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 1.0f };

        // Separators
        colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 0.67f };
        colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 1.0f };

        // Frame Background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.1569f, 0.1725f, 0.2039f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.1669f, 0.1825f, 0.2139f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.0835f, 0.0913f, 0.1070f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.1216f, 0.1373f, 0.1608f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.1669f, 0.1825f, 0.2139f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.0835f, 0.0913f, 0.1070f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.0835f, 0.0913f, 0.1070f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.1569f, 0.1725f, 0.2039f, 1.0f };

        colors[ImGuiCol_TitleBg] = ImVec4{ 0.0835f, 0.0913f, 0.1070f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.0835f, 0.0913f, 0.1070f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.0835f, 0.0913f, 0.1070f, 1.0f };

        // Window
        colors[ImGuiCol_SliderGrab] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 0.67f };
        colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 1.0f };

        colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 0.2f };
        colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 0.67f };
        colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.5543f, 0.6863f, 0.9373f, 1.0f };
    }

}
