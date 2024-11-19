#pragma once

#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Core/Handle.h"

#include <entt.hpp>

namespace Deako {

    using EntityHandle = DkHandle;

    class Entity
    {
    public:
        Entity() = default;
        Entity(EntityHandle handle, entt::entity enttEntity, const DkVec4& pickerColor, Scene* scene);
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) const
        {
            DK_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
            DK_CORE_ASSERT(m_Scene, "Scene is expired or null!1");

            T& component = m_Scene->GetRegistry().emplace<T>(m_EnttEntity, std::forward<Args>(args)...);

            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args) const
        {
            DK_CORE_ASSERT(m_Scene, "Scene is expired or null!2");

            T& component = m_Scene->GetRegistry().emplace_or_replace<T>(m_EnttEntity, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);

            return component;
        }

        template<typename T>
        T& GetComponent() const
        {
            DK_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            DK_CORE_ASSERT(m_Scene, "Scene is expired or null!3");

            return m_Scene->GetRegistry().get<T>(m_EnttEntity);
        }

        template<typename T>
        static T& GetComponent(EntityHandle handle)
        {
            DK_CORE_ASSERT(HasComponent<T>(handle), "Entity does not have component!");

            entt::entity enttEntity = Deako::GetActiveScene().GetEnttEntityMap().at(handle);
            return Deako::GetActiveScene().GetRegistry().get<T>(enttEntity);
        }

        template<typename T>
        bool HasComponent() const
        {
            DK_CORE_ASSERT(m_Scene, "Scene is expired or null!4");
            return m_Scene->GetRegistry().all_of<T>(m_EnttEntity);
        }

        template<typename T>
        static bool HasComponent(EntityHandle handle)
        {
            entt::entity enttEntity = Deako::GetActiveScene().GetEnttEntityMap().at(handle);
            return Deako::GetActiveScene().GetRegistry().all_of<T>(enttEntity);
        }

        template<typename T>
        void RemoveComponent() const
        {
            DK_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            DK_CORE_ASSERT(m_Scene, "Scene is expired or null!5");

            m_Scene->GetRegistry().remove<T>(m_EnttEntity);
        }

        EntityHandle GetHandle() { return m_EntityHandle; }
        const DkVec4& GetPickerColor() { return m_PickerColor; }
        const std::string& GetName() { return GetComponent<TagComponent>().tag; }

        // allows us to check if entity is valid, e.g., if(entity)
        operator bool() const { return m_EnttEntity != entt::null; }
        operator entt::entity() const { return m_EnttEntity; }
        operator DkU32() const { return (DkU32)m_EnttEntity; }
        bool operator==(const Entity& other) const { return m_EnttEntity == other.m_EnttEntity; }
        bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        EntityHandle m_EntityHandle = 0;
        entt::entity m_EnttEntity{ entt::null };
        DkVec4 m_PickerColor;

        Scene* m_Scene;
    };

}
