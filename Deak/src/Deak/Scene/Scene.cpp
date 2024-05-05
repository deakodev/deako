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

    void Scene::OnUpdate(Timestep timestep)
    {
        SceneCamera* mainCamera = nullptr;
        SceneCamera* hudCamera = nullptr;
        glm::mat4* cameraTransform = nullptr; // main and hud cameras share this transform
        {
            auto view = m_Registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
                const auto& [cameraComp, transformComp] = view.get<CameraComponent, TransformComponent>(entity);

                if (cameraComp.primary)
                {
                    mainCamera = &cameraComp.camera;
                    cameraTransform = &transformComp.transform;
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
            Renderer::BeginScene(*mainCamera, *cameraTransform);

            {
                auto group = m_Registry.group<ColorComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    const auto& [colorComp, transformComp] = group.get<ColorComponent, TransformComponent>(entity);

                    Renderer3D::DrawCube(transformComp, colorComp);
                }
            }

            {
                auto group = m_Registry.group<TextureComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    const auto& [textureComp, transformComp] = group.get<TextureComponent, TransformComponent>(entity);

                    Renderer3D::DrawCube(transformComp, textureComp.texture, 1.0f, glm::vec4(1.0f));
                }
            }

            Renderer::EndScene();
        }

        // Heads-Up Display - orthographic camera shadows/follows the main camera's transform
        if (hudCamera)
        {
            Renderer::BeginScene(*hudCamera, *cameraTransform);

            {
                auto group = m_Registry.group<OverlayComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    const auto& [overlayComp, transformComp] = group.get<OverlayComponent, TransformComponent>(entity);

                    glm::mat4 quadTransform = (*cameraTransform) * transformComp.transform;

                    Renderer2D::DrawQuad(quadTransform, overlayComp.color);
                }
            }

            Renderer::EndScene();
        }
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
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

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };

        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.tag = name.empty() ? "Entity" : name;

        return entity;
    }

}
