#include "Scene.h"
#include "dkpch.h"

#include "Entity.h"
#include "Components.h"
#include "Deako/Renderer/Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "System/Vulkan/VulkanBase.h"

namespace Deako {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::Prepare()
    {
        auto group = m_Registry.group<TransformComponent>(entt::get<ModelComponent>);
        for (auto entity : group)
        {
            auto [transformComp, modelComp] = group.get<TransformComponent, ModelComponent>(entity);

            VulkanBase::LoadModel(modelComp.relativePath);
        }
    }

    void Scene::OnUpdateEditor(Camera& editorCamera)
    {
        Renderer::BeginScene();

        {
            // auto group = m_Registry.group<TransformComponent>(entt::get<ModelComponent>);
            // for (auto entity : group)
            // {
            //     auto [transformComp, modelComp] = group.get<TransformComponent, ModelComponent>(entity);

            //     Renderer2D::DrawSprite(modelComp, transformComp.GetTransform());

            //     ForwardEntity(modelComp, transformComp.GetTransform());
            // }

            VulkanBase::UpdateUniforms();
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

    template<>
    void Scene::OnComponentAdded<ModelComponent>(Entity entity, ModelComponent& component)
    {
    }

}
