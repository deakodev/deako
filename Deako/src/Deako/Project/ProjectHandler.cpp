#include "ProjectHandler.h"
#include "dkpch.h"

#include "Deako/Asset/Scene/SceneHandler.h"
#include "Deako/Project/Serialize.h"
#include "Deako/Project/Deserialize.h"

#include "Deako/Core/Application.h" 
#include "System/MacOS/MacUtils.h" 

namespace Deako {

    void ProjectHandler::Init()
    {
        OpenProject();
        DK_CORE_ASSERT(!s_ActiveProject->projectFilename.empty(), "No project opened!");
    }

    void ProjectHandler::CleanUp()
    {
        SaveProject();
    }

    void ProjectHandler::OpenProject()
    {
        if (!s_ActiveProject->isSavedUpToDate)
        {
            bool continueToOpenProject = PromptToSaveProject();
            if (!continueToOpenProject) return; // user canceled
        }

        PromptToOpenProject();
    }

    void ProjectHandler::PromptToOpenProject()
    {
        std::filesystem::path projectPath = MacUtils::File::Open("dproj", "Open Project");
        if (!projectPath.empty()) OpenProject(projectPath);
    }

    void ProjectHandler::OpenProject(std::filesystem::path path)
    {
        Deserialize::Project(*s_ActiveProject, path) ?
            DK_CORE_INFO("Opened project <{0}>", path.filename().string()) :
            DK_CORE_WARN("Could not open project <{0}>", path.filename().string());

        DK_CORE_ASSERT(s_ActiveProject);
    }

    void ProjectHandler::SaveProject(std::filesystem::path path)
    {
        Serialize::Project(*s_ActiveProject, path) ?
            DK_CORE_INFO("Saved project <{0}>", path.filename().string()) :
            DK_CORE_WARN("Could not save project <{0}>", path.filename().string());
    }

    void ProjectHandler::SaveProject()
    {
        // set initial scene for next time project is accessed
        Ref<Scene> activeScene = SceneHandler::GetActiveScene();
        s_ActiveProject->initialSceneHandle = activeScene->m_Handle;

        if (s_ActiveProject->projectFilename.empty())
        {
            SaveAsProject();
            return;
        }

        std::filesystem::path projectPath = s_ActiveProject->workingDirectory / s_ActiveProject->projectFilename;

        SaveProject(projectPath);
    }

    void ProjectHandler::SaveAsProject()
    {
        std::filesystem::path projectPath = MacUtils::File::SaveAs("dproj", "Save Project");
        s_ActiveProject->projectFilename = projectPath.filename();

        SaveProject(projectPath);
    }

    bool ProjectHandler::PromptToSaveProject()
    {
        UserResponse response = (UserResponse)MacUtils::File::PromptSave("Do you want to save project changes?");

        return HandleUserResponse(response);
    }

    bool ProjectHandler::HandleUserResponse(UserResponse response)
    {
        switch (response)
        {
        case UserResponse::Save: { SaveProject(); return true; }
        case UserResponse::DontSave: { return true; }
        case UserResponse::Cancel: { return false; }
        }
    }

}
