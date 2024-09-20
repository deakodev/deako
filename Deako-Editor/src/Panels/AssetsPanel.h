#pragma once

#include "Deako.h"

namespace Deako {

    struct DirectoryEntry
    {
        bool isDirectory;
        std::filesystem::path path;
        std::string filename;
    };

    class AssetsPanel
    {
    public:
        AssetsPanel() = default;
        AssetsPanel(const Ref<Project>& project);

        void SetContext(const Ref<Project>& context);

        void OnImGuiRender();

        void Refresh();

    private:
        Ref<Project> m_Context;
        std::filesystem::path m_AssetDirectory;
        std::filesystem::path m_CurrentDirectory;
        std::vector<DirectoryEntry> m_DirectoryCache;

        RateLimiter m_RateLimiter;
    };

}
