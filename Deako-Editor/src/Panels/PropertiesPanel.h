#pragma once

#include "Deako.h"

namespace Deako {

    class PropertiesPanel
    {
    public:
        void OnImGuiRender();

    private:
        template<typename T, typename UIFunction>
        void DrawComponent(const std::string& name, const Entity& entity, ProjectAssetPool& assetPool, UIFunction uiFunction);
        void DrawComponents(const Entity& entity, ProjectAssetPool& assetPool);
    };

}
