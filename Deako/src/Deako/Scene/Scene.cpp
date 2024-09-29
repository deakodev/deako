#include "Scene.h"
#include "dkpch.h"

#include "Deako/Asset/Prefab.h"
#include "Deako/Renderer/Renderer.h"
#include "Deako/Project/Serialize.h"

namespace Deako {

    Ref<Scene> Scene::Open(const std::filesystem::path& path)
    {
        s_ActiveScene = AssetPool::Import<Scene>(path);

        return s_ActiveScene;
    }

    bool Scene::Save()
    {
        if (Serialize::Scene(*this))
            return true;

        return false;
    }

    void Scene::OnUpdate()
    {
        Renderer::BeginScene();

        Renderer::EndScene();
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };

        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto& tagComp = entity.AddComponent<TagComponent>();
        tagComp.tag = name.empty() ? "Entity" : name;

        m_EntityMap[uuid] = entity;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    Entity Scene::GetEntity(entt::entity handle)
    {
        auto entityHandles = m_Registry.view<TagComponent>();
        for (auto entityHandle : entityHandles)
        {
            if (entityHandle == handle)
                return { entityHandle, this };
        }
        return {};
    }

    Entity Scene::GetEntity(const std::string& tag)
    {
        auto entityHandles = m_Registry.view<TagComponent>();
        for (auto entityHandle : entityHandles)
        {
            const TagComponent& tagComp = entityHandles.get<TagComponent>(entityHandle);
            if (tagComp.tag == tag)
                return { entityHandle, this };
        }
        return {};
    }

    Entity Scene::GetEntity(UUID uuid)
    {
        if (m_EntityMap.find(uuid) != m_EntityMap.end())
            return { m_EntityMap.at(uuid), this };

        return {};
    }

    template<>
    std::vector<Entity> Scene::GetAllEntitiesWith<TagComponent, ModelComponent>()
    {
        auto entityHandles = s_ActiveScene->m_Registry.view<TagComponent, ModelComponent>();

        std::vector<Entity> entities;
        for (auto entityHandle : entityHandles)
        {
            entities.emplace_back(entityHandle, s_ActiveScene.get());
        }

        return entities;
    }

    template<>
    std::vector<Entity> Scene::GetAllEntitiesWith<PrefabComponent>()
    {
        auto entityHandles = s_ActiveScene->m_Registry.view<PrefabComponent>();

        std::vector<Entity> entities;
        for (auto entityHandle : entityHandles)
        {
            entities.emplace_back(entityHandle, s_ActiveScene.get());
        }

        return entities;
    }

    void Scene::LinkAssets()
    {
        std::vector<Entity> prefabEntities = GetAllEntitiesWith<PrefabComponent>();
        for (auto prefabEntity : prefabEntities)
        {
            AssetHandle prefabHandle = prefabEntity.GetComponent<PrefabComponent>().handle;
            Ref<Prefab> prefabAsset = AssetPool::GetAsset<Prefab>(prefabHandle);

            // Assign prefab asset textures
              // TODO: need to add support for multiple textures
            if (prefabAsset->textures.size() > 0)
            {
                if (!prefabEntity.HasComponent<TextureComponent>())
                    prefabEntity.AddComponent<TextureComponent>();

                AssetHandle& textureHandle = prefabEntity.GetComponent<TextureComponent>().handle;
                textureHandle = prefabAsset->textures[0]->m_Handle;
            }

            // Assign prefab asset materials
            if (prefabAsset->materials.size() > 0)
            {
                if (!prefabEntity.HasComponent<MaterialComponent>())
                    prefabEntity.AddComponent<MaterialComponent>();

                auto& entityMaterialHandles = prefabEntity.GetComponent<MaterialComponent>().handles;
                for (const auto& [assetHandle, materialAsset] : prefabAsset->materials)
                    entityMaterialHandles.emplace_back(assetHandle);
            }

            // Assign prefab asset model
            if (prefabAsset->model)
            {
                if (!prefabEntity.HasComponent<ModelComponent>())
                    prefabEntity.AddComponent<ModelComponent>(prefabAsset->model->m_Handle);
            }
        }
    }

    template<>
    void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
    {
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
