#pragma once

namespace Deako {

    class AssetManager
    {
    public:

        static void SetTexturePath(const std::string& relativePath);
        static const std::vector<std::string>& GetTexturePaths() { return s_TexturePaths; }

    private:
        static std::string s_BasePath;
        static std::vector<std::string> s_TexturePaths;

    };

}
