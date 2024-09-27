#include "SceneImporter.h"
#include "dkpch.h"

#include "Deako/Project/Deserialize.h"

namespace Deako {

    Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, AssetMetadata metadata)
    {
        if (metadata.assetPath.extension().string() != ".dscene")
        {
            DK_WARN("Could not import scene from <{0}>", metadata.assetPath.filename().string());
            return nullptr;
        }

        Ref<Scene> scene = Deserialize::Scene(metadata.assetPath);

        if (scene) return scene;
        else return nullptr;
    }

}
