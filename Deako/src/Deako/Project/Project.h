#pragma once

#include "Deako/Asset/Asset.h"

namespace Deako {

    struct Project
    {
        std::string name{ "Untitled" };

        std::filesystem::path workingDirectory;
        std::filesystem::path assetDirectory;

        std::filesystem::path projectFilename;
        std::filesystem::path assetRegistryFilename;

        AssetHandle initialSceneHandle;

        bool isSavedUpToDate{ true };
    };

}
