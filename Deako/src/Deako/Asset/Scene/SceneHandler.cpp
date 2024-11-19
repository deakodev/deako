#include "SceneHandler.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/EditorAssetPool.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Project/ProjectHandler.h"

#include "Deako/Project/Serialize.h"
#include "Deako/Project/Deserialize.h"

#include "System/MacOS/MacUtils.h" 

namespace Deako {

    void SceneHandler::Init()
    {
        DkContext& deako = Deako::GetContext();

        AssetHandle initialSceneHandle = deako.activeProject->initialSceneHandle;
        SetActiveScene(initialSceneHandle);

        // empty scene
        AssetHandle handle = 0;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Scene;
        metadata.assetPath = "Deako/assets/empty/emptyScene.dscene";

        s_EmptyScene = ImportScene(handle, metadata);
    }

    void SceneHandler::CleanUp()
    {
        // SaveScene();
    }

    void SceneHandler::NewScene()
    {
        DkContext& deako = Deako::GetContext();

        Ref<Scene> newScene = Scene::Copy(s_EmptyScene);

        AssetMetadata metadata;
        metadata.assetType = AssetType::Scene;
        metadata.assetPath = MacUtils::File::SaveAs("dscene", "New Scene");
        std::string assetName = metadata.assetPath.filename().string();
        assetName[0] = std::toupper(assetName[0]);
        metadata.assetName = assetName;

        if (!metadata.assetPath.empty())
        {
            SaveScene(metadata);
            deako.projectAssetPool->AddAssetToPool(newScene, metadata);

            SetActiveScene(newScene);
        }
    }

    void SceneHandler::OpenScene()
    {
        Scene& activeScene = Deako::GetActiveScene();

        if (!activeScene.IsSceneSaved())
        {
            bool continueToOpenScene = PromptToSaveScene();
            if (!continueToOpenScene) return; // user canceled, so exit
        }

        PromptToOpenScene();
    }

    void SceneHandler::PromptToOpenScene()
    {
        std::filesystem::path scenePath = MacUtils::File::Open("dscene", "Open Scene");
        if (!scenePath.empty()) ImportScene(scenePath);
    }

    void SceneHandler::ImportScene(std::filesystem::path path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Scene;
        metadata.assetPath = path;

        Ref<Scene> scene = ImportScene(handle, metadata);
        SetActiveScene(scene);
    }

    Ref<Scene> SceneHandler::ImportScene(AssetHandle handle, AssetMetadata& metadata)
    {
        DkContext& deako = Deako::GetContext();

        DK_CORE_INFO("Importing Scene <{0}>", metadata.assetPath.filename().string());

        // Ref<Scene> readOnlyScene = CreateRef<Scene>();
        Ref<Scene> newScene = CreateRef<Scene>();

        Deserialize::Scene(*newScene, metadata);

        // newScene = Scene::Copy(readOnlyScene);
        newScene->m_Handle = handle;
        std::string assetName = metadata.assetPath.filename().string();
        assetName[0] = std::toupper(assetName[0]);
        metadata.assetName = assetName;

        if (assetName != "EmptyScene.dscene")
            deako.projectAssetPool->AddAssetToPool(newScene, metadata);
        else
            deako.editorAssetPool->AddAssetToPool(newScene);

        return newScene;
    }

    void SceneHandler::SaveScene(AssetMetadata& metadata)
    {
        Scene& activeScene = Deako::GetActiveScene();

        Serialize::Scene(activeScene, metadata) ?
            DK_CORE_INFO("Saved scene <{0}>", metadata.assetPath.filename().string()) :
            DK_CORE_WARN("Could not save scene <{0}>", metadata.assetPath.filename().string());
    }

    void SceneHandler::SaveScene()
    {
        DkContext& deako = Deako::GetContext();

        AssetMetadata& sceneMetadata = deako.projectAssetPool->GetAssetMetadata(deako.activeSceneHandle);

        if (sceneMetadata.assetPath.empty())
        {
            SaveAsScene();
            return;
        }

        SaveScene(sceneMetadata);
    }

    void SceneHandler::SaveAsScene()
    {
        DkContext& deako = Deako::GetContext();

        AssetMetadata& sceneMetadata = deako.projectAssetPool->GetAssetMetadata(deako.activeSceneHandle);
        sceneMetadata.assetPath = MacUtils::File::SaveAs("dscene", "Save Scene");

        SaveScene(sceneMetadata);
    }

    bool SceneHandler::PromptToSaveScene()
    {
        UserResponse response = (UserResponse)MacUtils::File::PromptSave("Do you want to save scene changes?");

        return HandleUserResponse(response);
    }

    void SceneHandler::RefreshScene()
    {
        Scene& activeScene = Deako::GetActiveScene();
        activeScene.LinkAssets();
    }

    bool SceneHandler::HandleUserResponse(UserResponse response)
    {
        switch (response)
        {
        case UserResponse::Save: { SaveScene(); return true; }
        case UserResponse::DontSave: { return true; }
        case UserResponse::Cancel: { return false; }
        }
    }

}
