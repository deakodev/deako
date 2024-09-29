#include "AssetsPanel.h"
#include "dkpch.h"

#include <imgui/imgui.h>

namespace Deako {

    static RegistryBins s_RegistryBins;
    static std::map<AssetType, RegistryBin*> s_RegistryBinMap = {
        { AssetType::Texture2D, &s_RegistryBins.Texture },
        { AssetType::TextureCubeMap, &s_RegistryBins.Texture },
        { AssetType::Material, &s_RegistryBins.Material  },
        { AssetType::Model, &s_RegistryBins.Model  },
        { AssetType::Prefab, &s_RegistryBins.Prefab  },
        { AssetType::Scene, &s_RegistryBins.Scene  }
    };

    AssetsPanel::AssetsPanel(const Ref<Project>& context)
        : m_RateLimiter(0, 1) // 0 counter cycle, 1-second time cycle
    {
        SetContext(context);
    }

    void AssetsPanel::SetContext(const Ref<Project>& context)
    {
        m_Context = context;
        m_AssetDirectory = Project::GetActive()->GetAssetDirectory();
        m_CurrentDirectory = m_AssetDirectory;
        RefreshBrowser();
        RefreshRegistry();
    }

    void AssetsPanel::OnImGuiRender()
    {
        auto now = RateLimiter::Clock::now();

        ImGui::Begin("Assets");

        if (m_RateLimiter.Trigger(now))
        {
            RefreshBrowser();
            // DK_INFO("Refresh time: {0}", TimePointToString(now));
        }

        if (m_CurrentDirectory != std::filesystem::path(m_AssetDirectory))
        {
            if (ImGui::ArrowButton("##Back", ImGuiDir_Left))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
                RefreshBrowser(); // refresh when exiting a directory
            }
        }

        ImGui::SameLine();

        if (ImGui::BeginTabBar("Asset Tabs", ImGuiTabBarFlags_None))
        {
            OnBrowserTabRender();
            OnRegistryTabRender();
            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void AssetsPanel::OnBrowserTabRender()
    {
        float buttonWidth = ImGui::GetContentRegionAvail().x;
        float buttonHeight = 22.0f;

        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4{ 0.1f, 0.25f, 0.8f, 0.5f });
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 0.5f });
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4{ 0.1f, 0.25f, 0.8f, 0.5f });
        if (ImGui::BeginTabItem("Browser"))
        {
            if (ImGui::BeginTable("Entries", 1))
            {
                ImGui::TableNextColumn();

                for (const BrowserItem& item : m_BrowserCache)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    const std::string& icon = item.isDirectory ? Icons::Folder : Icons::File;
                    std::string label = icon + "  " + item.filename;

                    ImGui::Button(label.c_str(), { buttonWidth, buttonHeight });
                    if (ImGui::BeginDragDropSource())
                    {
                        std::string itemPath = item.path.string();
                        ImGui::SetDragDropPayload("ASSET_PATH", itemPath.c_str(), itemPath.size() + 1, ImGuiCond_Once);
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (item.isDirectory)
                        {
                            m_CurrentDirectory /= item.path.filename();
                            RefreshBrowser(); // refresh when entering a directory
                        }
                    }
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
        ImGui::PopStyleColor(3);
    }

    void AssetsPanel::OnRegistryTabRender()
    {
        float buttonWidth = ImGui::GetContentRegionAvail().x;
        float buttonHeight = 22.0f;

        if (ImGui::BeginTabItem("Registry"))
        {
            if (ImGui::BeginTabBar("Registry Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Textures"))
                {
                    for (auto& [handle, name] : s_RegistryBins.Texture)
                    {
                        ImGui::Button(name.c_str(), { buttonWidth, buttonHeight });
                        if (ImGui::BeginDragDropSource())
                        {
                            ImGui::SetDragDropPayload("ASSET_PATH", &handle, sizeof(AssetHandle));
                            ImGui::EndDragDropSource();
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Materials"))
                {
                    ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Models"))
                {
                    ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Prefabs"))
                {
                    ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Scenes"))
                {
                    ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
    }

    void AssetsPanel::RefreshBrowser()
    {
        m_BrowserCache.clear();

        for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = it.path();
            std::string filename = std::filesystem::relative(path, m_AssetDirectory).filename().string();

            if (filename.front() == '.')
                continue; // skip hidden files

            m_BrowserCache.push_back({ it.is_directory(), path, filename });
        }
    }

    void AssetsPanel::RefreshRegistry()
    {
        const auto& assetRegistry = AssetPool::GetAssetRegistry();
        for (const auto& [handle, metadata] : assetRegistry)
        {
            auto it = s_RegistryBinMap.find(metadata.assetType);

            if (it != s_RegistryBinMap.end())
            {
                it->second->emplace_back(handle, metadata.assetPath.filename().string());
            }
        }
    }

}
