#pragma once

namespace Deako {

    class AssetManager
    {
    public:

        static void SetTexturePath(const std::string& relativePath);
        static const std::vector<std::string>& GetTexture2DPaths() { return s_Texture2DPaths; }

        static void SetModelPath(const std::string& relativePath);
        static const std::vector<std::string>& GetModelPaths() { return s_ModelPaths; }

    private:
        static std::string s_BasePath;
        static std::vector<std::string> s_Texture2DPaths;
        static std::vector<std::string> s_ModelPaths;

    };

}
