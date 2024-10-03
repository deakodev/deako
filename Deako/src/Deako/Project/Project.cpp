#include "Project.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/AssetManager.h"
#include "Deako/Project/Serialize.h"
#include "Deako/Project/Deserialize.h"

#include "Deako/Core/Application.h"

namespace Deako {

    void Project::Init()
    {
        std::filesystem::path projectPath = Application::Get().GetSpecification().commandLineArgs[1];
        DK_CORE_ASSERT(projectPath.extension().string() == ".dproj", "Invalid project path: <{0}>", projectPath.filename().string());

        s_ActiveProject = Deserialize::Project(projectPath);
    }

    void Project::PrepareScene()
    {
        AssetHandle sceneHandle = s_ActiveProject->GetInitialSceneHandle();
        s_ActiveScene = AssetManager::GetProjectAssetPool()->GetAsset<Scene>(sceneHandle);

        if (!s_ActiveScene)
        {
            Ref<Asset> emptyScene = AssetManager::GetEditorAssetPool()->GetEmpty(AssetType::Scene);
            s_ActiveScene = std::static_pointer_cast<Scene>(emptyScene);
        }

        s_ActiveScene->LinkAssets();
    }

    void Project::PrepareScene(AssetHandle sceneHandle)
    {
        s_ActiveScene = AssetManager::GetProjectAssetPool()->GetAsset<Scene>(sceneHandle);

        if (!s_ActiveScene)
        {
            Ref<Asset> emptyScene = AssetManager::GetEditorAssetPool()->GetEmpty(AssetType::Scene);
            s_ActiveScene = std::static_pointer_cast<Scene>(emptyScene);
        }

        s_ActiveScene->LinkAssets();
    }

    void Project::CleanUp()
    {
    }

    Ref<Project> Project::Load(const std::filesystem::path& path)
    {
        if (path.extension().string() != ".dproj")
        {
            DK_WARN("Could not load project from <{0}>", path.filename().string());
            return nullptr;
        }

        s_ActiveProject = Deserialize::Project(path);
        return s_ActiveProject;
    }

    Ref<Scene> Project::LoadScene(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Scene;
        metadata.assetPath = path;

        Ref<Asset> asset = AssetManager::GetProjectAssetPool()->ImportAsset(handle, metadata);
        s_ActiveScene = std::static_pointer_cast<Scene>(asset);
        s_ActiveScene->LinkAssets();
        return s_ActiveScene;
    }

    bool Project::Save()
    {
        if (Serialize::Project(*s_ActiveProject) && Serialize::Scene(*s_ActiveScene))
            return true;

        return false;
    }

    bool Project::SaveScene()
    {
        if (Serialize::Scene(*s_ActiveScene))
            return true;

        return false;
    }

}
