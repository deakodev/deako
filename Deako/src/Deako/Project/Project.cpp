#include "Project.h"
#include "dkpch.h"

#include "Deako/Asset/Prefab.h"
#include "Deako/Project/Serialize.h"

namespace Deako {

    Ref<Project> Project::Open(const std::filesystem::path& path)
    {
        std::filesystem::path fullPath = s_ProjectDirectory / path;

        s_ActiveProject = Deserialize::Project(fullPath);

        if (s_ActiveProject)
        {
            Ref<AssetRegistry> assetRegistry = Deserialize::AssetRegistry();
            s_ActiveProject->m_AssetPool = CreateRef<AssetPoolBase>(assetRegistry);

            Ref<Scene> firstScene;

            for (auto& [handle, metadata] : *assetRegistry)
            {
                metadata.assetPath = GetAssetDirectory() / metadata.assetPath;
                Ref<Asset> asset = AssetPool::Import(handle, metadata);

                if (!firstScene && metadata.assetType == AssetType::Scene)
                    firstScene = std::dynamic_pointer_cast<Scene>(asset);
            }

            if (firstScene)
            {
                Scene::SetActive(firstScene);
            }
            else
            {
                DK_CORE_ERROR("No active scene selected!");
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
