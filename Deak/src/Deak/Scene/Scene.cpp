#include "Scene.h"
#include "dkpch.h"

#include "Entity.h"
#include "Components.h"
#include "Deak/Renderer/Renderer.h"
#include "Deak/Renderer/Renderer2D.h"
#include "Deak/Renderer/Renderer3D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Deak {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::OnUpdateEditor(Timestep timestep, EditorCameraController& cameraController)
    {
        Renderer::BeginScene(cameraController);

        {
            auto group = m_Registry.group<ColorComponent>(entt::get<TransformComponent>);
            for (auto entity : group)
            {
                auto [colorComp, transformComp] = group.get<ColorComponent, TransformComponent>(entity);

                Renderer3D::DrawCube(transformComp.GetTransform(), colorComp);
            }
        }

        {
            auto group = m_Registry.group<TextureComponent>(entt::get<TransformComponent>);
            for (auto entity : group)
            {
                auto [textureComp, transformComp] = group.get<TextureComponent, TransformComponent>(entity);

                Renderer3D::DrawCube(transformComp.GetTransform(), textureComp.texture, 1.0f, glm::vec4(1.0f));
            }
        }

        {
            auto group = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
            for (auto entity : group)
            {
                auto [spriteRendererComp, transformComp] = group.get<SpriteRendererComponent, TransformComponent>(entity);

                Renderer3D::DrawCube(transformComp.GetTransform(), spriteRendererComp.color);
            }
        }

        Renderer::EndScene();
    }

    void Scene::OnUpdateRuntime(Timestep timestep)
    {
        // Update Scripts
        m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nativeScriptComp)
            {
                //TODO: move to scene play
                if (!nativeScriptComp.instance)
                {
                    nativeScriptComp.instance = nativeScriptComp.InstantiateScript();
                    nativeScriptComp.instance->m_Entity = Entity{ entity, this };
                    nativeScriptComp.instance->OnCreate();
                }

                nativeScriptComp.instance->OnUpdate(timestep);
            });


        SceneCamera* mainCamera = nullptr;
        SceneCamera* hudCamera = nullptr;
        glm::mat4 cameraTransform; // main and hud cameras share this transform
        {
            auto view = m_Registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [cameraComp, transformComp] = view.get<CameraComponent, TransformComponent>(entity);

                if (cameraComp.primary)
                {
                    mainCamera = &cameraComp.camera;
                    cameraTransform = transformComp.GetTransform();
                }

                if (cameraComp.hud)
                {
                    hudCamera = &cameraComp.camera;
                }
            }
        }

        // Game world view - perspective for now
        if (mainCamera)
        {
            Renderer::BeginScene(*mainCamera, cameraTransform);

            {
                auto group = m_Registry.group<ColorComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    auto [colorComp, transformComp] = group.get<ColorComponent, TransformComponent>(entity);

                    Renderer3D::DrawCube(transformComp.GetTransform(), colorComp);
                }
            }

            {
                auto group = m_Registry.group<TextureComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    auto [textureComp, transformComp] = group.get<TextureComponent, TransformComponent>(entity);

                    Renderer3D::DrawCube(transformComp.GetTransform(), textureComp.texture, 1.0f, glm::vec4(1.0f));
                }
            }

            {
                auto group = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    auto [spriteRendererComp, transformComp] = group.get<SpriteRendererComponent, TransformComponent>(entity);

                    Renderer3D::DrawCube(transformComp.GetTransform(), spriteRendererComp.color);
                }
            }

            Renderer::EndScene();
        }

        // Heads-Up Display - orthographic camera shadows/follows the main camera's transform
        if (hudCamera && mainCamera)
        {
            Renderer::BeginScene(*hudCamera, cameraTransform);

            {
                auto group = m_Registry.group<OverlayComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    auto [overlayComp, transformComp] = group.get<OverlayComponent, TransformComponent>(entity);

                    glm::mat4 quadTransform = cameraTransform * transformComp.GetTransform();

                    Renderer2D::DrawQuad(quadTransform, overlayComp.texture, 1.0f, overlayComp.color);
                }
            }

            Renderer::EndScene();
        }
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

    void Scene::OnViewportResize(float width, float height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComp = view.get<CameraComponent>(entity);
            if (!cameraComp.fixedAspectRatio)
            {
                cameraComp.camera.SetViewportSize(width, height);
            }
        }
    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            const auto& cameraComp = view.get<CameraComponent>(entity);
            if (cameraComp.primary)
                return Entity{ entity, this };
        }

        return {};
    }

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        // static_assert(false);
    }

    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        component.camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
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
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<OverlayComponent>(Entity entity, OverlayComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TextureComponent>(Entity entity, TextureComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<ColorComponent>(Entity entity, ColorComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
    {
    }

}
