#include "Scene.h"
#include "dkpch.h"

#include "Deako/Renderer/Renderer.h"
#include "Deako/Project/Serialize.h"

#include "Entity.h"

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

    std::unordered_map<std::string, Ref<Model>> Scene::GetModels()
    {
        auto modalEntities = m_Registry.view<TagComponent, ModelComponent>();

        std::unordered_map<std::string, Ref<Model>> models;

        for (auto& entity : modalEntities)
        {
            auto [tagComp, modelComp] = modalEntities.get<TagComponent, ModelComponent>(entity);
            models[tagComp.tag] = modelComp.model;
        }

        return models;
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
