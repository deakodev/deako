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
        RegistryPanel();

        void OnImGuiRender();
        void OnRegistryRender();

        void Refresh();

    private:
        RateLimiter m_RateLimiter;
    };

}
