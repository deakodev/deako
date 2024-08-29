#include "ContentBrowserPanel.h"
#include "dkpch.h"

#include <imgui/imgui.h>

namespace Deako {

    ContentBrowserPanel::ContentBrowserPanel(const Ref<Project>& context)
        : m_RateLimiter(0, 1) // 0 counter cycle, 1-second time cycle
    {
        SetContext(context);
    }

    void ContentBrowserPanel::SetContext(const Ref<Project>& context)
    {
        m_Context = context;
        m_AssetDirectory = Project::GetAssetDirectory();
        m_CurrentDirectory = m_AssetDirectory;
        Refresh();
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        auto now = RateLimiter::Clock::now();

        ImGui::Begin("Content Browser");

        if (m_RateLimiter.Trigger(now))
        {
            Refresh();
            // DK_INFO("Refresh time: {0}", TimePointToString(now));
        }

        if (m_CurrentDirectory != std::filesystem::path(m_AssetDirectory))
        {
            if (ImGui::ArrowButton("##Back", ImGuiDir_Left))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
                Refresh(); // refresh when exiting a directory
            }
        }

        static float cellMargin = 10.0f;
        static float thumbnailSize = 92.0f;
        float cellWidth = thumbnailSize + cellMargin;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellWidth);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(cellMargin, 0.0f));

        // render from cached directory contents
        for (const DirectoryEntry& entry : m_DirectoryCache)
        {
            auto& icon = entry.isDirectory ? Icons::Folder : Icons::File;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

            ImGui::Button(icon, { thumbnailSize, thumbnailSize });

            ImGui::PopStyleColor(3);

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (entry.isDirectory)
                {
                    m_CurrentDirectory /= entry.path.filename();
                    Refresh(); // refresh when entering a directory
                }
            }

            ImGui::TextWrapped("%s", entry.filename.c_str());

            ImGui::NextColumn();
        }

        ImGui::PopStyleVar();
        ImGui::Columns(1);

        ImGui::End();
    }

    void ContentBrowserPanel::Refresh()
    {
        m_DirectoryCache.clear(); // Clear previous contents

        for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = it.path();
            std::string filename = std::filesystem::relative(path, m_AssetDirectory).filename().string();

            if (filename.front() == '.')
                continue; // skip hidden files

            m_DirectoryCache.push_back({ it.is_directory(), path, filename });
        }
    }

}
