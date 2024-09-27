#pragma once

#include "Deako/Asset/AssetPool.h"

namespace Deako {

    struct ProjectMetadata
    {
        std::string name{ "Untitled" };

        std::filesystem::path workingDirectory;
        std::filesystem::path assetDirectory;

        std::filesystem::path assetRegistryPath;
        std::filesystem::path initialScenePath;
    };

    class Project
    {
    public:
        static Ref<Project> Load(const std::filesystem::path& path);
        bool Save();

        static Ref<Project> GetActive() { return s_ActiveProject; }

        std::filesystem::path GetWorkingDirectory()
        {
            return m_Metadata.workingDirectory;
        }

        std::filesystem::path GetAssetDirectory()
        {
            return GetWorkingDirectory() / m_Metadata.assetDirectory;
        }

        std::filesystem::path GetAssetRegistryPath()
        {
            return GetWorkingDirectory() / m_Metadata.assetRegistryPath;
        }

        std::filesystem::path GetInitialScenePath()
        {
            return GetAssetDirectory() / m_Metadata.initialScenePath;
        }

        void SetMetadata(const ProjectMetadata& metadata) { m_Metadata = metadata; }
        const ProjectMetadata& GetMetadata() { return m_Metadata; }

        Ref<AssetPoolBase> GetAssetPool() { return m_AssetPool; }

    private:
        ProjectMetadata m_Metadata;
        Ref<AssetPoolBase> m_AssetPool;

        inline static Ref<Project> s_ActiveProject;
    };

}
