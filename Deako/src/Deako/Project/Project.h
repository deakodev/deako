#pragma once

namespace Deako {

    struct ProjectDetails
    {
        std::string name{ "Untitled" };
        std::filesystem::path path;
        std::filesystem::path firstScene;
    };

    class Project
    {
    public:
        Project(const std::filesystem::path& path);

        static Ref<Project> Open(const std::string& filename);
        bool Save();

        const std::filesystem::path& GetFirstScenePath() { return m_Details.firstScene; }

        void SetDetails(const ProjectDetails& details) { m_Details = details; }
        const ProjectDetails& GetDetails() { return m_Details; }

    private:
        ProjectDetails m_Details;

        inline static Ref<Project> s_ActiveProject;
    };


}
