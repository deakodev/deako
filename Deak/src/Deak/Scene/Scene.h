#pragma once

#include "Deak/Core/Timestep.h"
#include "entt.hpp"

namespace Deak {

    class Entity;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void OnUpdate(Timestep timestep);

        Entity CreateEntity(const std::string& name = std::string());

    private:
        entt::registry m_Registry;

        friend class Entity;
    };

}
