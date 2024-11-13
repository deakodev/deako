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
        DkContext& deako = Deako::GetContext();
        OpenProject();
        DK_CORE_ASSERT(!deako.activeProject->projectFilename.empty(), "No project opened!");
    }

    void ProjectHandler::CleanUp()
    {
        SaveProject();
    }

    void ProjectHandler::OpenProject()
    {
        DkContext& deako = Deako::GetContext();

        if (!deako.activeProject->isSavedUpToDate)
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
        DkContext& deako = Deako::GetContext();

        Deserialize::Project(*deako.activeProject, path) ?
            DK_CORE_INFO("Opened project <{0}>", path.filename().string()) :
            DK_CORE_WARN("Could not open project <{0}>", path.filename().string());

        DK_CORE_ASSERT(deako.activeProject, "No active project!");
    }

    void ProjectHandler::SaveProject(std::filesystem::path path)
    {
        DkContext& deako = Deako::GetContext();

        Serialize::Project(*deako.activeProject, path) ?
            DK_CORE_INFO("Saved project <{0}>", path.filename().string()) :
            DK_CORE_WARN("Could not save project <{0}>", path.filename().string());
    }

    void ProjectHandler::SaveProject()
    {
        DkContext& deako = Deako::GetContext();

        // set initial scene for next time project is accessed
        deako.activeProject->initialSceneHandle = deako.activeSceneHandle;

        if (deako.activeProject->projectFilename.empty())
        {
            SaveAsProject();
            return;
        }

        std::filesystem::path projectPath = deako.activeProject->workingDirectory / deako.activeProject->projectFilename;

        SaveProject(projectPath);
    }

    void ProjectHandler::SaveAsProject()
    {
        DkContext& deako = Deako::GetContext();

        std::filesystem::path projectPath = MacUtils::File::SaveAs("dproj", "Save Project");
        deako.activeProject->projectFilename = projectPath.filename();

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
