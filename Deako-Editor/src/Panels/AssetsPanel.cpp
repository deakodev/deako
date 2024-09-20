#include "AssetsPanel.h"
#include "dkpch.h"

#include <imgui/imgui.h>

namespace Deako {

    AssetsPanel::AssetsPanel(const Ref<Project>& context)
        : m_RateLimiter(0, 1) // 0 counter cycle, 1-second time cycle
    {
        SetContext(context);
    }

    void AssetsPanel::SetContext(const Ref<Project>& context)
    {
        m_Context = context;
        m_AssetDirectory = Project::GetAssetDirectory();
        m_CurrentDirectory = m_AssetDirectory;
        Refresh();
    }

    void AssetsPanel::OnImGuiRender()
    {
        auto now = RateLimiter::Clock::now();

        ImGui::Begin("Assets");

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

        float buttonWidth = ImGui::GetContentRegionAvail().x;
        float buttonHeight = 22.0f;

        if (ImGui::BeginTable("Entries", 1))
        {
            ImGui::TableNextColumn();

            for (const DirectoryEntry& entry : m_DirectoryCache)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                auto& icon = entry.isDirectory ? Icons::Folder : Icons::File;

                ImGui::Button((icon + entry.filename).c_str(), { buttonWidth, buttonHeight });

                if (ImGui::BeginDragDropSource())
                {
                    std::string itemPath = entry.path.string();
                    ImGui::SetDragDropPayload("ASSET_PATH", itemPath.c_str(), itemPath.size() + 1, ImGuiCond_Once);
                    ImGui::EndDragDropSource();
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (entry.isDirectory)
                    {
                        m_CurrentDirectory /= entry.path.filename();
                        Refresh(); // refresh when entering a directory
                    }
                }
            }
            ImGui::EndTable();
        }

        // render from cached directory contents
        // for (const DirectoryEntry& entry : m_DirectoryCache)
        // {
        //     auto& icon = entry.isDirectory ? Icons::Folder : Icons::File;

        //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        //     ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        //     ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

        //     ImGui::Button((icon + entry.filename).c_str(), { thumbnailSize, thumbnailSize });

        //     ImGui::PopStyleColor(3);

        //     if (ImGui::BeginDragDropSource())
        //     {
        //         std::string itemPath = entry.path.string();
        //         ImGui::SetDragDropPayload("ASSET_PATH", itemPath.c_str(), itemPath.size() + 1, ImGuiCond_Once);
        //         ImGui::EndDragDropSource();
        //     }

        //     if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        //     {
        //         if (entry.isDirectory)
        //         {
        //             m_CurrentDirectory /= entry.path.filename();
        //             Refresh(); // refresh when entering a directory
        //         }
        //     }

        //     // ImGui::TextWrapped("%s", entry.filename.c_str());

        //     ImGui::NextColumn();
        // }

        // ImGui::PopStyleVar();
        // ImGui::Columns(1);

        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void AssetsPanel::Refresh()
    {
        m_DirectoryCache.clear();

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
