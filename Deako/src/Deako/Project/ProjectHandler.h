#pragma once

#include "Deako/Project/Project.h"

namespace Deako {

    enum class UserResponse { Save = 0, DontSave = 1, Cancel = 2 };

    class ProjectHandler
    {
    public:
        static void Init();
        static void CleanUp();

        static void OpenProject();
        static void SaveProject();
        static void SaveAsProject();

        static bool IsProjectSaved()
        {
            DkContext& deako = Deako::GetContext();
            return deako.activeProject->isSavedUpToDate;
        }

    private:
        static void OpenProject(std::filesystem::path path);
        static void SaveProject(std::filesystem::path path);
        static void PromptToOpenProject();
        static bool PromptToSaveProject();

        static bool HandleUserResponse(UserResponse response);
    };

}
