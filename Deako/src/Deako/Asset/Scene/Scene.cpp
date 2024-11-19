#include "Scene.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Prefab/Prefab.h"
#include "Deako/Project/Serialize.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"

namespace Deako {

    using EnttEnttMap = std::unordered_map<entt::entity, entt::entity>;

    template<typename... Component>
    static void CopyComponent(
        entt::registry& dstRegistry,
        entt::registry& srcRegistry,
        const EnttEnttMap& enttEnttMap)
    {
        ([&]()
            {
                auto srcEnttEntities = srcRegistry.view<Component>();
                for (auto srcEnttEntity : srcEnttEntities)
                {
                    auto dstEnttEntityIt = enttEnttMap.find(srcEnttEntity);
                    if (dstEnttEntityIt == enttEnttMap.end())
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
        const EnttEnttMap& enttEnttMap)
    {
        CopyComponent<Component...>(dstRegistry, srcRegistry, enttEnttMap);
    }

    void Scene::Build()
    {
        m_VulkanScene = CreateScope<VulkanScene>(this);
        m_VulkanScene->Build();
        m_IsValid = true;
    }

    void Scene::Rebuild()
    {
        DK_CORE_ASSERT(m_VulkanScene, "VulkanScene has not be created!");
        m_VulkanScene->Rebuild();
        m_IsValid = true;
    }

    void Scene::CleanUp()
    {
        m_VulkanScene->CleanUp();
    }

    void Scene::OnUpdate()
    {
        if (m_ActiveCamera)
            m_ActiveCamera->OnUpdate();

        if (!m_IsValid)
        {
            this->Rebuild();
            return;
        }

        m_VulkanScene->OnUpdate();
    }

    Ref<Scene> Scene::Copy(Ref<Scene> srcScene)
    {
        Ref<Scene> dstScene = CreateRef<Scene>();
        std::unordered_map<entt::entity, entt::entity> enttEnttMap;

        // Create entities in dstScene and store mapping from src to dst entities
        for (const auto& [handle, srcEnttEntity] : srcScene->GetEnttEntityMap())
        {
            const auto& name = srcScene->GetRegistry().get<TagComponent>(srcEnttEntity).tag;
            Entity dstEnttEntity = dstScene->CreateEntity(name, handle);
            enttEnttMap[srcEnttEntity] = (entt::entity)dstEnttEntity; // Map src to dst entity
        }

        // Copy components (except TagComponent)
        CopyComponent(AllComponents{}, dstScene->GetRegistry(), srcScene->GetRegistry(), enttEnttMap);

        return dstScene;
    }

    Entity Scene::CreateEntity(const std::string& name, EntityHandle handle)
    {
        entt::entity enttEntity = m_Registry.create();
        DkU32 colorOffset = 1;
        DkVec4 pickerColor = { U32ToVec3((DkU32)enttEntity + colorOffset), 1.0f };
        Entity entity = { handle, enttEntity, pickerColor, this };

        entity.AddComponent<TransformComponent>();
        auto& tagComp = entity.AddComponent<TagComponent>();
        tagComp.tag = name.empty() ? "Entity" : name;

        m_Entities.push_back(entity);
        m_EnttEntityMap[handle] = (entt::entity)entity;
        m_PickerColorMap[pickerColor] = handle;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    Entity Scene::GetEntity(EntityHandle handle)
    {
        for (auto& entity : m_Entities)
        {
            if (entity.GetHandle() == handle)
                return entity;
        }

        return {};
    }

    EntityHandle Scene::GetEntityHandle(const DkVec4& pickerColor)
    {
        if (m_PickerColorMap.find(pickerColor) != m_PickerColorMap.end())
            return m_PickerColorMap.at(pickerColor);

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
    //         auto it = std::find_if(m_EnttEntityMap.begin(), m_EnttEntityMap.end(), [enttEntity](const auto& pair)
    //             {
    //                 return pair.second == enttEntity;
    //             });

    //         if (it != m_EnttEntityMap.end())
    //             entities.emplace_back(it->first, enttEntity, this);
    //     }

    //     return entities;
    // }

    // template std::vector<Entity> Scene::GetAllEntitiesWith<TagComponent>();
    // template std::vector<Entity> Scene::GetAllEntitiesWith<PrefabComponent>();

    void Scene::LinkAssets()
    {
        ProjectAssetPool& projectAssetPool = Deako::GetProjectAssetPool();

        for (auto& entity : m_Entities)
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

    template<>
    void Scene::OnComponentAdded<TransformComponent>(const Entity& entity, TransformComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(const Entity& entity, TagComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TextureComponent>(const Entity& entity, TextureComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<MaterialComponent>(const Entity& entity, MaterialComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<ModelComponent>(const Entity& entity, ModelComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<PrefabComponent>(const Entity& entity, PrefabComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<EnvironmentComponent>(const Entity& entity, EnvironmentComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CameraComponent>(const Entity& entity, CameraComponent& component)
    {
        component.camera = m_ActiveCamera;
    }


}
