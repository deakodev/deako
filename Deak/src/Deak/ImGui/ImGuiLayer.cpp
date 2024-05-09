#include "ImGuiLayer.h"
#include "dkpch.h"

#include "Deak/Core/Application.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <GLFW/glfw3.h>
namespace Deak {

    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    void ImGuiLayer::OnAttach()
    {
        DK_PROFILE_FUNC();

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io.Fonts->AddFontFromFileTTF("Deak-Editor/assets/fonts/roboto/Roboto-Bold.ttf", 16.0f);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("Deak-Editor/assets/fonts/roboto/Roboto-Regular.ttf", 16.0f);

        // If viewports enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        style.WindowMenuButtonPosition = ImGuiDir_None;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
        SetDarkThemeColors();

        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::OnDetach()
    {
        DK_PROFILE_FUNC();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnEvent(Event& event)
    {
        if (m_BlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }

    void ImGuiLayer::Begin()
    {
        DK_PROFILE_FUNC();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuizmo::BeginFrame();
    }

    void ImGuiLayer::End()
    {
        DK_PROFILE_FUNC();

        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
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
