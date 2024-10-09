#pragma once

#include "Deako.h"
#include "../EditorContext.h"

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
        RegistryPanel(Ref<EditorContext> editorContext);

        void OnImGuiRender();
        void OnRegistryRender();

        void Refresh();

    private:
        Ref<EditorContext> m_EditorContext;
        RateLimiter m_RateLimiter;
    };

}
