#include "Scene.h"
#include "dkpch.h"

#include "Deako/Renderer/Renderer.h"
#include "System/Vulkan/VulkanBase.h"

#include "Entity.h"
#include "Serialize.h"

namespace Deako {

    Scene::Scene(const std::filesystem::path& path)
    {
        m_Details.path = path;
    }

    Ref<Scene> Scene::Open(const std::string& filename)
    {
        std::filesystem::path path = "Deako-Editor/projects/" + filename;

        Ref<Scene> scene = Deserialize::Scene(path);

        if (scene)
        {
            s_ActiveScene = scene;
            return s_ActiveScene;
        }

        return nullptr;
    }

    bool Scene::Save()
    {
        if (Serialize::Scene(*this))
            return true;

        return false;
    }

    void Scene::Prepare()
    {
        auto group = m_Registry.group<TransformComponent>(entt::get<ModelComponent>);
        for (auto entity : group)
        {
            auto [transformComp, modelComp] = group.get<TransformComponent, ModelComponent>(entity);

            VulkanBase::LoadModel(modelComp.path);
        }
    }

    void Scene::OnUpdateEditor(Camera& editorCamera)
    {
        Renderer::BeginScene();

        {
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

    std::vector<std::string> Scene::GetModelPaths()
    {
        auto modalEntities = m_Registry.view<ModelComponent>();
        std::vector<std::string> modelPaths;

        for (auto& entity : modalEntities)
        {
            auto& modelComp = modalEntities.get<ModelComponent>(entity);
            modelPaths.push_back(modelComp.path);
        }

        return modelPaths;
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
