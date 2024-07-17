#include "AssetManager.h"
#include "dkpch.h"

namespace Deako {

    std::string AssetManager::s_BasePath{ "Deako-Editor/assets" };
    std::vector<std::string> AssetManager::s_Texture2DPaths;
    std::vector<std::string> AssetManager::s_ModelPaths;

    void AssetManager::SetTexturePath(const std::string& relativePath)
    {
        std::string path = s_BasePath + relativePath;
        s_Texture2DPaths.push_back(path);
    }

    void AssetManager::SetModelPath(const std::string& relativePath)
    {
        std::string path = s_BasePath + relativePath;
        s_ModelPaths.push_back(path);
    }

}
