#include "Scene.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Prefab/Prefab.h"
#include "Deako/Renderer/Renderer.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"
#include "Deako/Project/Serialize.h"

namespace Deako {

    template<typename... Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        ([&]()
            {
                auto view = src.view<Component>();
                for (auto srcEntity : view)
                {
                    entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).id);

                    auto& srcComponent = src.get<Component>(srcEntity);
                    dst.emplace_or_replace<Component>(dstEntity, srcComponent);
                }
            }(), ...);
    }

    template<typename... Component>
    static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        CopyComponent<Component...>(dst, src, enttMap);
    }

    template<typename... Component>
    static void CopyComponentIfExists(Entity dst, Entity src)
    {
        ([&]()
            {
                if (src.HasComponent<Component>())
                    dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
            }(), ...);
    }

    template<typename... Component>
    static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
    {
        CopyComponentIfExists<Component...>(dst, src);
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = CreateRef<Scene>();

        auto& srcSceneRegistry = other->registry;
        auto& dstSceneRegistry = newScene->registry;
        std::unordered_map<UUID, entt::entity> enttMap;

        // create entities in new scene
        auto entityHandles = srcSceneRegistry.view<IDComponent>();
        for (auto entityHandle : entityHandles)
        {
            UUID uuid = srcSceneRegistry.get<IDComponent>(entityHandle).id;
            const auto& name = srcSceneRegistry.get<TagComponent>(entityHandle).tag;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            enttMap[uuid] = (entt::entity)newEntity;
        }

        // Copy components (except IDComponent and TagComponent)
        CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

        return newScene;
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
        Entity entity = { registry.create(), this };

        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto& tagComp = entity.AddComponent<TagComponent>();
        tagComp.tag = name.empty() ? "Entity" : name;

        entityMap[uuid] = entity;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        registry.destroy(entity);
    }

    Entity Scene::GetEntity(entt::entity handle)
    {
        auto entityHandles = registry.view<TagComponent>();
        for (auto entityHandle : entityHandles)
        {
            if (entityHandle == handle)
                return { entityHandle, this };
        }
        return {};
    }

    Entity Scene::GetEntity(const std::string& tag)
    {
        auto entityHandles = registry.view<TagComponent>();
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
        if (entityMap.find(uuid) != entityMap.end())
            return { entityMap.at(uuid), this };

        return {};
    }

    uint32_t Scene::GetSelectedEntity()
    {
        return VulkanScene::GetSelectedEntityID();
    }

    template<typename... Components>
    std::vector<Entity> Scene::GetAllEntitiesWith()
    {
        auto entityHandles = registry.view<Components...>();

        std::vector<Entity> entities;
        for (auto entityHandle : entityHandles)
        {
            entities.emplace_back(entityHandle, this);
        }

        return entities;
    }

    template std::vector<Entity> Scene::GetAllEntitiesWith<TagComponent>();
    template std::vector<Entity> Scene::GetAllEntitiesWith<PrefabComponent>();

    void Scene::LinkAssets()
    {
        std::vector<Entity> prefabEntities = GetAllEntitiesWith<PrefabComponent>();
        for (auto prefabEntity : prefabEntities)
        {
            auto& prefabComp = prefabEntity.GetComponent<PrefabComponent>();
            Ref<Prefab> prefabAsset = ProjectAssetPool::Get()->GetAsset<Prefab>(prefabComp.handle);

            // Assign prefab asset textures
            if (prefabAsset)
            {
                if (prefabAsset->textures.size() > 0)
                {
                    prefabComp.textureHandles.reserve(prefabAsset->textures.size());
                    for (const auto& [assetHandle, materialAsset] : prefabAsset->textures)
                        prefabComp.textureHandles.emplace_back(assetHandle);
                }

                // Assign prefab asset materials
                if (prefabAsset->materials.size() > 0)
                {
                    prefabComp.textureHandles.reserve(prefabAsset->materials.size());
                    for (const auto& [assetHandle, materialAsset] : prefabAsset->materials)
                        prefabComp.materialHandles.emplace_back(assetHandle);
                }

                // Assign prefab asset model
                if (prefabAsset->model)
                    prefabComp.meshHandle = prefabAsset->model->m_Handle;
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
