#include "Entity.h"
#include "dkpch.h"

namespace Deak {

    Entity::Entity(entt::entity handle, Scene* scene)
        : m_EntityHandle(handle), m_Scene(scene)
    {
    }

}
