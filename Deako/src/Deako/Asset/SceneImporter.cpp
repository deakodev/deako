#include "SceneImporter.h"
#include "dkpch.h"

#include "Deako/Project/Serialize.h"

namespace Deako {

    Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
    {
        DK_CORE_INFO("Importing Scene <{0}>", metadata.path.filename().string());

        Ref<Scene> scene = Deserialize::Scene(metadata.path);

        if (scene) return scene;
        else return nullptr;
    }

}
