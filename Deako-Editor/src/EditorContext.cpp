#include "EditorContext.h"

namespace Deako {

    EditorContext::EditorContext()
    {
        Ref<Scene> activeScene = SceneHandler::GetActiveScene();
        scene.Set(activeScene);

        Ref<ProjectAssetPool> projectAssetPool = ProjectAssetPool::Get();
        assetPool.Set(projectAssetPool);

        Ref<Project> activeProject = ProjectHandler::GetActiveProject();
        project.Set(activeProject);
    }

    void EditorContext::OnUpdate()
    {
        // if (!entity.isValid)
        // {
        //     //Get new enetity
        //     entity.Set(nullptr);
        // }
        if (!scene.isValid)
        {
            Ref<Scene> activeScene = SceneHandler::GetActiveScene();
            scene.Set(activeScene);
        }
        if (!assetPool.isValid)
        {
            Ref<ProjectAssetPool> projectAssetPool = ProjectAssetPool::Get();
            assetPool.Set(projectAssetPool);
        }
        if (!project.isValid)
        {
            Ref<Project> activeProject = ProjectHandler::GetActiveProject();
            project.Set(activeProject);
        }
    }

}
