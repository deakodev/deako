#include "Scene.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Prefab/Prefab.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"
#include "Deako/Project/Serialize.h"

namespace Deako {

    using EnttEntityMap = std::unordered_map<entt::entity, entt::entity>;

    template<typename... Component>
    static void CopyComponent(
        entt::registry& dstRegistry,
        entt::registry& srcRegistry,
        const EnttEntityMap& enttEntityMap)
    {
        ([&]()
            {
                auto srcEnttEntities = srcRegistry.view<Component>();
                for (auto srcEnttEntity : srcEnttEntities)
                {
                    auto dstEnttEntityIt = enttEntityMap.find(srcEnttEntity);
                    if (dstEnttEntityIt == enttEntityMap.end())
                        continue; // Skip if the dstEnttEntity wasn't found

                    entt::entity dstEnttEntity = dstEnttEntityIt->second;
                    auto& srcComponent = srcRegistry.get<Component>(srcEnttEntity);
                    dstRegistry.emplace_or_replace<Component>(dstEnttEntity, srcComponent);
                }
            }(), ...);
    }

    template<typename... Component>
    static void CopyComponent(
        ComponentGroup<Component...>,
        entt::registry& dstRegistry,
        entt::registry& srcRegistry,
        const EnttEntityMap& enttEntityMap)
    {
        CopyComponent<Component...>(dstRegistry, srcRegistry, enttEntityMap);
    }

    Ref<Scene> Scene::Copy(Ref<Scene> srcScene)
    {
        Ref<Scene> dstScene = CreateRef<Scene>();
        std::unordered_map<entt::entity, entt::entity> enttEntityMap;

        // Create entities in dstScene and store mapping from src to dst entities
        for (const auto& [handle, srcEnttEntity] : srcScene->entityMap)
        {
            const auto& name = srcScene->registry.get<TagComponent>(srcEnttEntity).tag;
            Entity dstEnttEntity = dstScene->CreateEntity(name, handle);
            enttEntityMap[srcEnttEntity] = (entt::entity)dstEnttEntity; // Map src to dst entity
        }

        // Copy components (except TagComponent)
        CopyComponent(AllComponents{}, dstScene->registry, srcScene->registry, enttEntityMap);

        return dstScene;
    }

    void Scene::OnUpdate()
    {
        if (activeCamera)
            activeCamera->OnUpdate();

        if (!Deako::GetActiveScene().isValid)
        {
            VulkanScene::Rebuild(); return;
        }

        VulkanScene::OnUpdate();
    }

    Entity Scene::CreateEntity(const std::string& name, EntityHandle handle)
    {
        // After calling registry.create()
        entt::entity enttEntity = registry.create();
        DkU32 colorOffset = 1;
        DkVec4 pickerColor = { U32ToVec3((DkU32)enttEntity + colorOffset), 1.0f };
        Entity entity = { handle, enttEntity, pickerColor, this };

        entity.AddComponent<TransformComponent>();
        auto& tagComp = entity.AddComponent<TagComponent>();
        tagComp.tag = name.empty() ? "Entity" : name;

        entities.push_back(entity);
        entityMap[handle] = (entt::entity)entity;
        pickerColorMap[pickerColor] = handle;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        registry.destroy(entity);
    }

    Entity Scene::GetEntity(EntityHandle handle)
    {
        for (auto& entity : entities)
        {
            if (entity.GetHandle() == handle)
                return entity;
        }

        return {};
    }

    EntityHandle Scene::GetEntityHandle(const DkVec4& pickerColor)
    {
        if (pickerColorMap.find(pickerColor) != pickerColorMap.end())
            return pickerColorMap.at(pickerColor);

        return 0;
    }

    // template<typename... Components>
    // std::vector<Entity> Scene::GetAllEntitiesWith()
    // {
    //     auto enttEntities = registry.view<Components...>();

    //     std::vector<Entity> entities;
    //     for (auto enttEntity : enttEntities)
    //     {
    //         // find the EntityHandle associated with this enttEntity
    //         auto it = std::find_if(entityMap.begin(), entityMap.end(), [enttEntity](const auto& pair)
    //             {
    //                 return pair.second == enttEntity;
    //             });

    //         if (it != entityMap.end())
    //             entities.emplace_back(it->first, enttEntity, this);
    //     }

    //     return entities;
    // }

    // template std::vector<Entity> Scene::GetAllEntitiesWith<TagComponent>();
    // template std::vector<Entity> Scene::GetAllEntitiesWith<PrefabComponent>();

    void Scene::LinkAssets()
    {
        ProjectAssetPool& projectAssetPool = Deako::GetProjectAssetPool();
        Scene& activeScene = Deako::GetActiveScene();

        for (auto& entity : activeScene.entities)
        {
            if (entity.HasComponent<PrefabComponent>())
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Prefab> prefabAsset = projectAssetPool.GetAsset<Prefab>(prefabComp.handle);

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
