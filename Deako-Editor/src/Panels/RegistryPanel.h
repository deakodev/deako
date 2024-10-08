#pragma once

#include "Deako.h"

namespace Deako {

    using RegistryItem = std::pair<AssetHandle, std::string>;
    using RegistryBin = std::vector<RegistryItem>;

    struct RegistryBins
    {
        RegistryBin Texture;
        RegistryBin Material;
        RegistryBin Model;
        RegistryBin Prefab;
        RegistryBin Scene;
    };

    struct BrowserItem
    {
        bool isDirectory;
        std::filesystem::path path;
        std::string filename;
    };

    class RegistryPanel
    {
    public:
        RegistryPanel() = default;
        RegistryPanel(Ref<Project> project, Ref<ProjectAssetPool> projectAssetPool);

        void SetContext(Ref<Project> project, Ref<ProjectAssetPool> projectAssetPool);

        void OnImGuiRender();
        void OnRegistryRender();

        void Refresh();

    private:
        Ref<Project> m_ProjectContext;
        Ref<ProjectAssetPool> m_ProjectAssetPool;

        RateLimiter m_RateLimiter;
    };

}
