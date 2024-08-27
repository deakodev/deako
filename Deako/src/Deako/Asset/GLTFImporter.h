#pragma once

#include "Asset.h"

#include "Deako/Renderer/Vulkan/VulkanModel.h"

namespace Deako {

    class GLTFImporter
    {
    public:
        static Ref<Model> ImportGLTF(const std::filesystem::path& path);

    private:

    };

}
