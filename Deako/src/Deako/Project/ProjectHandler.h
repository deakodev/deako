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

        static Ref<Project> GetActiveProject() { return s_ActiveProject; }
        static bool IsProjectSaved() { return s_ActiveProject->isSavedUpToDate; }

    private:
        static void OpenProject(std::filesystem::path path);
        static void SaveProject(std::filesystem::path path);
        static void PromptToOpenProject();
        static bool PromptToSaveProject();

        static bool HandleUserResponse(UserResponse response);

    private:
        inline static Ref<Project> s_ActiveProject = CreateRef<Project>();
    };

}
