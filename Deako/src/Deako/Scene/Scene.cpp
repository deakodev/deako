#include "Scene.h"
#include "dkpch.h"

#include "Entity.h"
#include "Components.h"
#include "Deako/Renderer/Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deako {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::OnUpdateEditor(const glm::mat4& viewProjection)
    {
        Renderer::BeginScene(viewProjection);

        {
            // auto group = m_Registry.group<TextureComponent>(entt::get<TransformComponent>);
            // for (auto entity : group)
            // {
            //     auto [textureComp, transformComp] = group.get<TextureComponent, TransformComponent>(entity);

            //     Renderer::PrepareModelInstance(transformComp.translation, transformComp.rotation, transformComp.scale, 0);
            // }
        }

        Renderer::EndScene();
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };

        entity.AddComponent<TransformComponent>();
        auto& tagComp = entity.AddComponent<TagComponent>();
        tagComp.tag = name.empty() ? "Entity" : name;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TextureComponent>(Entity entity, TextureComponent& component)
    {
    }

}
