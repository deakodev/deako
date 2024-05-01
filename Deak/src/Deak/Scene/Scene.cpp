#include "Scene.h"
#include "dkpch.h"

#include "Entity.h"
#include "Components.h"
#include "Deak/Renderer/Renderer3D.h"

#include <glm/glm.hpp>

namespace Deak {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::OnUpdate(Timestep timestep)
    {
        auto group = m_Registry.group<TransformComponent>(entt::get<TextureComponent>);
        for (auto entity : group)
        {
            const auto& [transform, texture2D] = group.get<TransformComponent, TextureComponent>(entity);

            Renderer3D::DrawCube(transform, texture2D.texture, 1.0f, glm::vec4(1.0f));
        }
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };

        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.tag = name.empty() ? "Entity" : name;

        return entity;
    }

}
