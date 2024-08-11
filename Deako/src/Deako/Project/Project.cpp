#include "Project.h"
#include "dkpch.h"

#include "Serialize.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Deako {

    Project::Project(const std::filesystem::path& path)
    {
        m_Details.path = path;
    }

    Ref<Project> Project::Open(const std::string& filename)
    {
        std::filesystem::path path = std::filesystem::path("Deako-Editor/projects/") / filename;

        Ref<Project> project = Deserialize::Project(path);

        if (project)
        {
            s_ActiveProject = project;
            return s_ActiveProject;
        }

        return nullptr;
    }

    bool Project::Save()
    {
        if (Serialize::Project(*this))
            return true;

        return false;
    }

}
