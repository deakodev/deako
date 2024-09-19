#include "Project.h"
#include "dkpch.h"

#include "Deako/Asset/Prefab.h"
#include "Deako/Project/Serialize.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Deako {

    Ref<Project> Project::Open(const std::filesystem::path& path)
    {
        std::filesystem::path fullPath = s_ProjectDirectory / path;

        Ref<Project> project = Deserialize::Project(fullPath);

        if (project)
        {
            s_ActiveProject = project;

            Ref<AssetRegistry> assetRegistry = Deserialize::AssetRegistry();
            s_ActiveProject->m_AssetPool = CreateRef<AssetPoolBase>(assetRegistry);

            for (auto& [handle, metadata] : *assetRegistry)
            {
                metadata.assetPath = GetAssetDirectory() / metadata.assetPath;
                Ref<Asset> asset = s_ActiveProject->m_AssetPool->Import(handle, metadata);
            }

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
