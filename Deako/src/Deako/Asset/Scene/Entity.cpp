#include "Entity.h"
#include "dkpch.h"

namespace Deako {

    Entity::Entity(EntityHandle handle, entt::entity enttEntity, const DkVec4& pickerColor, Scene* scene)
        : m_EntityHandle(handle), m_EnttEntity(enttEntity), m_PickerColor(pickerColor), m_Scene(scene)
    {
    }

}
