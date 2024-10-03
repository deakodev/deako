#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Scene/Scene.h"

namespace Deako {

    struct ProjectMetadata
    {
        std::string name{ "Untitled" };

        std::filesystem::path workingDirectory;
        std::filesystem::path assetDirectory;

        std::filesystem::path assetRegistryPath;
        AssetHandle initialSceneHandle;
    };

    class Project
    {
    public:
        static void Init();
        static void CleanUp();

        static Ref<Project> Load(const std::filesystem::path& path);
        bool Save();

        static Ref<Scene> LoadScene(const std::filesystem::path& path);
        static void PrepareScene();
        static void PrepareScene(AssetHandle sceneHandle);
        bool SaveScene();

        static Ref<Project> GetActive() { return s_ActiveProject; }
        static Ref<Scene> GetActiveScene() { return s_ActiveScene; }

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

        AssetHandle GetInitialSceneHandle()
        {
            return m_Metadata.initialSceneHandle;
        }

        void SetMetadata(const ProjectMetadata& metadata) { m_Metadata = metadata; }
        const ProjectMetadata& GetProjectMetadata() { return m_Metadata; }

    private:
        ProjectMetadata m_Metadata;

        inline static Ref<Scene> s_ActiveScene;
        inline static Ref<Project> s_ActiveProject;
    };

}
