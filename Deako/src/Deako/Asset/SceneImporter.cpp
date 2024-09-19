#include "SceneImporter.h"
#include "dkpch.h"

#include "Deako/Project/Serialize.h"

namespace Deako {

    Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, AssetMetadata metadata)
    {
        DK_CORE_INFO("Importing Scene <{0}>", metadata.assetPath.filename().string());

        Ref<Scene> scene = Deserialize::Scene(metadata.assetPath);

        if (scene) return scene;
        else return nullptr;
    }

}
