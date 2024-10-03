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

    class AssetsPanel
    {
    public:
        AssetsPanel() = default;
        AssetsPanel(Ref<Project> project, Ref<ProjectAssetPool> projectAssetPool);

        void SetContext(Ref<Project> project, Ref<ProjectAssetPool> projectAssetPool);

        void OnImGuiRender();
        void OnBrowserTabRender();
        void OnRegistryTabRender();

        void RefreshBrowser();
        void RefreshRegistry();

    private:
        Ref<Project> m_ProjectContext;
        Ref<ProjectAssetPool> m_ProjectAssetPool;

        std::filesystem::path m_AssetDirectory;
        std::filesystem::path m_CurrentDirectory;
        std::vector<BrowserItem> m_BrowserCache;

        RateLimiter m_RateLimiter;
    };

}
