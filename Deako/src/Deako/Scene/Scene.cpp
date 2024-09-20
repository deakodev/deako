#include "Scene.h"
#include "dkpch.h"

#include "Deako/Renderer/Renderer.h"
#include "Deako/Renderer/Vulkan/VulkanBase.h"
#include "Deako/Project/Serialize.h"

#include "Entity.h"

namespace Deako {

    Scene::Scene(const std::filesystem::path& path)
    {
        m_Details.path = path;
    }

    Ref<Scene> Scene::Open(const std::filesystem::path& path)
    {
        Ref<Scene> scene = AssetPool::Import<Scene>(path);

        if (scene)
        {
            SetActive(scene);
            VulkanBase::UpdateScene();
            return scene;
        }

        return nullptr;
    }

    bool Scene::Save()
    {
        if (Serialize::Scene(*this))
            return true;

        return false;
    }

    void Scene::OnUpdateEditor(Camera& editorCamera)
    {
        Renderer::BeginScene();

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

    Entity Scene::GetEntityByName(const std::string& name)
    {
        auto entities = m_Registry.view<TagComponent>();
        for (auto entity : entities)
        {
            const TagComponent& tc = entities.get<TagComponent>(entity);
            if (tc.tag == name)
                return Entity{ entity, this };
        }
        return {};
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
    void Scene::OnComponentAdded<MaterialComponent>(Entity entity, MaterialComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<ModelComponent>(Entity entity, ModelComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<PrefabComponent>(Entity entity, PrefabComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<EnvironmentComponent>(Entity entity, EnvironmentComponent& component)
    {
    }


}
