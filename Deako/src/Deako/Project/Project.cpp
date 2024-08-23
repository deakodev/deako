#include "Project.h"
#include "dkpch.h"

#include "Deako/Project/Serialize.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Deako {

    Ref<Project> Project::Open(const std::string& filename)
    {
        std::filesystem::path path = s_ProjectDirectory / filename;

        Ref<Project> project = Deserialize::Project(path);

        if (project)
        {
            s_ActiveProject = project;

            Ref<AssetRegistry> assetRegistry = Deserialize::AssetRegistry();
            s_ActiveProject->m_AssetPool = CreateRef<EditorAssetPool>(assetRegistry);

            return s_ActiveProject;
        }

        return nullptr;
    }

    bool Project::Save()
    {
        if (Serialize::Project() && Serialize::AssetRegistry())
            return true;

        return false;
    }

}
