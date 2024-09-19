#pragma once

#include "Scene.h"

#include "Deako/Core/UUID.h"

#include "entt.hpp"

namespace Deako {

    using EntityID = UUID;

    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            DK_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T>
        T& GetComponent()
        {
            DK_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            DK_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        UUID GetUUID() { return GetComponent<IDComponent>().id; }
        const std::string& GetName() { return GetComponent<TagComponent>().tag; }

        // allows us to check if entity is valid, e.g., if(entity)
        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint32_t() const { return (uint32_t)m_EntityHandle; }
        bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
        bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        entt::entity m_EntityHandle{ entt::null };

        // we dont want the Scene to be owned by Entity, could potentially use a weak_ref too
        Scene* m_Scene = nullptr;
    };

}
