#pragma once

#include "Deak/Renderer/Texture.h"
#include "Deak/Scene/SceneCamera.h"
#include "Deak/Scene/ScriptableEntity.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deak {

    struct TagComponent
    {
        std::string tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : tag(tag)
        {
        }
    };

    struct TransformComponent
    {
        glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;

        glm::mat4 GetTransform() const
        {
            glm::mat4 _rotation = glm::toMat4(glm::quat(rotation));

            return glm::translate(glm::mat4(1.0f), translation)
                * _rotation * glm::scale(glm::mat4(1.0f), scale);
        }
    };

    struct ColorComponent
    {
        glm::vec4 color = glm::vec4(1.0f);

        ColorComponent() = default;
        ColorComponent(const ColorComponent&) = default;
        ColorComponent(const glm::vec4& color)
            : color(color) {}

        operator glm::vec4& () { return color; }
        operator const glm::vec4& () const { return color; }
    };

    struct TextureComponent
    {
        Ref<Texture2D> texture;

        TextureComponent() = default;
        TextureComponent(const TextureComponent&) = default;
        TextureComponent(const Ref<Texture2D>& texture)
            : texture(texture) {}
    };

    struct OverlayComponent // for now this is used for the heads-up display
    {
        Ref<Texture2D> texture = nullptr;
        glm::vec4 color = glm::vec4(1.0f); // default white color

        OverlayComponent() = default;
        OverlayComponent(const OverlayComponent&) = default;
        OverlayComponent(const glm::vec4& color)
            : color(color) {}
        OverlayComponent(const Ref<Texture2D>& texture, const glm::vec4& color = glm::vec4(1.0f))
            : texture(texture), color(color) {}
    };

    struct SpriteRendererComponent
    {
        Ref<Texture2D> texture = nullptr;
        glm::vec4 color = glm::vec4(1.0f); // default white color

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : color(color) {}
        SpriteRendererComponent(const Ref<Texture2D>& texture, const glm::vec4& color = glm::vec4(1.0f))
            : texture(texture), color(color) {}
    };

    struct CameraComponent
    {
        SceneCamera camera = { Perspective };
        bool primary = false;
        bool hud = false; // heads-up display
        bool fixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
        CameraComponent(ProjectionType projectionType)
            :camera(projectionType) {}
    };

    struct NativeScriptComponent
    {
        ScriptableEntity* instance = nullptr;

        ScriptableEntity* (*InstantiateScript)();
        void (*DestroyScript)(NativeScriptComponent*);

        template<typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->instance; nsc->instance = nullptr; };
        }

    };

}
