#include "Project.h"
#include "dkpch.h"

#include "Deako/Asset/Prefab.h"
#include "Deako/Project/Serialize.h"
#include "Deako/Project/Deserialize.h"

namespace Deako {

    Ref<Project> Project::Load(const std::filesystem::path& path)
    {
        if (path.extension().string() != ".dproj")
        {
            DK_WARN("Could not load project from <{0}>", path.filename().string());
            return nullptr;
        }

        s_ActiveProject = Deserialize::Project(path);

        if (s_ActiveProject)
        {
            const std::filesystem::path& assetRegistryPath = Project::GetActive()->GetAssetRegistryPath();
            Ref<AssetRegistry> assetRegistry = Deserialize::AssetRegistry(assetRegistryPath);
            s_ActiveProject->m_AssetPool = CreateRef<AssetPoolBase>(assetRegistry);

            return s_ActiveProject;
        }

        return nullptr;
    }

    bool Project::Save()
    {
        if (Serialize::Project(*this) && Serialize::AssetRegistry())
            return true;

        return false;
    }

}
