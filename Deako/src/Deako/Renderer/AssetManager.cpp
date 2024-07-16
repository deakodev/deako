#include "AssetManager.h"
#include "dkpch.h"

namespace Deako {

    std::string AssetManager::s_BasePath{ "Deako-Editor/assets" };
    std::vector<std::string> AssetManager::s_TexturePaths;

    void AssetManager::SetTexturePath(const std::string& relativePath)
    {
        std::string path = s_BasePath + relativePath;
        s_TexturePaths.push_back(path);
    }

}
