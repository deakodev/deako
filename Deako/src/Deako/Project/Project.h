#pragma once

#include "Deako/Asset/AssetPool.h"

namespace Deako {

    struct ProjectDetails
    {
        std::string name{ "Untitled" };

        std::filesystem::path assetDirectory;
        std::filesystem::path assetRegistryPath; // relative to assetDirectory
    };

    class Project
    {
    public:
        static Ref<Project> Open(const std::filesystem::path& path);
        bool Save();

        static Ref<Project> GetActive() { return s_ActiveProject; }

        static std::filesystem::path GetProjectDirectory()
        {
            DK_CORE_ASSERT(s_ActiveProject);
            return s_ProjectDirectory;
        }

        static std::filesystem::path GetAssetDirectory()
        {
            DK_CORE_ASSERT(s_ActiveProject);
            return GetProjectDirectory() / s_ActiveProject->m_Details.assetDirectory;
        }

        static std::filesystem::path GetAssetRegistryPath()
        {
            DK_CORE_ASSERT(s_ActiveProject);
            return GetAssetDirectory() / s_ActiveProject->m_Details.assetRegistryPath;
        }


        void SetDetails(const ProjectDetails& details) { m_Details = details; }
        const ProjectDetails& GetDetails() { return m_Details; }

        Ref<AssetPoolBase> GetAssetPool() { return m_AssetPool; }

    private:
        ProjectDetails m_Details;
        Ref<AssetPoolBase> m_AssetPool;

        inline static std::filesystem::path s_ProjectDirectory{ "Deako-Editor/projects/" };
        inline static Ref<Project> s_ActiveProject;
    };

}
