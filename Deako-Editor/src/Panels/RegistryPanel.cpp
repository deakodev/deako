#include "RegistryPanel.h"
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

    RegistryPanel::RegistryPanel(Ref<Project> project, Ref<ProjectAssetPool> projectAssetPool)
        : m_RateLimiter(0, 1) // 0 counter cycle, 1-second time cycle
    {
        SetContext(project, projectAssetPool);
    }

    void RegistryPanel::SetContext(Ref<Project> project, Ref<ProjectAssetPool> projectAssetPool)
    {
        m_ProjectContext = project;
        m_ProjectAssetPool = projectAssetPool;
        Refresh();
    }

    void RegistryPanel::OnImGuiRender()
    {
        auto now = RateLimiter::Clock::now();

        ImGui::Begin("Assets");

        if (m_RateLimiter.Trigger(now))
        {
            Refresh();
            // DK_INFO("Refresh time: {0}", TimePointToString(now));
        }

        OnRegistryRender();

        ImGui::SameLine();

        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void RegistryPanel::OnRegistryRender()
    {
        float buttonWidth = ImGui::GetContentRegionAvail().x;
        float buttonHeight = 22.0f;

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
                for (auto& [handle, name] : s_RegistryBins.Material)
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

            if (ImGui::BeginTabItem("Models"))
            {
                for (auto& [handle, name] : s_RegistryBins.Model)
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

            if (ImGui::BeginTabItem("Prefabs"))
            {
                for (auto& [handle, name] : s_RegistryBins.Prefab)
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

            if (ImGui::BeginTabItem("Scenes"))
            {
                for (auto& [handle, name] : s_RegistryBins.Scene)
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
            ImGui::EndTabBar();
        }

    }

    void RegistryPanel::Refresh()
    {
        for (auto& [assetType, registryBin] : s_RegistryBinMap)
            registryBin->clear();

        const auto& registryAssets = m_ProjectAssetPool->GetAssetRegistry();
        for (const auto& [handle, metadata] : registryAssets)
        {
            auto it = s_RegistryBinMap.find(metadata.assetType);
            if (it != s_RegistryBinMap.end())
                it->second->emplace_back(handle, metadata.assetName);
        }
    }

}
