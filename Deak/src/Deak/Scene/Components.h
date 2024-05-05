#pragma once

#include "Deak/Renderer/Texture.h"
#include "Deak/Scene/SceneCamera.h"

#include <glm/glm.hpp>

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
        glm::mat4 transform = glm::mat4(1.0f);

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::mat4& transform)
            : transform(transform)
        {
        }

        operator glm::mat4& () { return transform; }
        operator const glm::mat4& () const { return transform; }
    };

    struct ColorComponent
    {
        glm::vec4 color;

        ColorComponent() = default;
        ColorComponent(const ColorComponent&) = default;
        ColorComponent(const glm::vec4& color)
            : color(color)
        {
        }

        operator glm::vec4& () { return color; }
        operator const glm::vec4& () const { return color; }
    };

    struct TextureComponent
    {
        Ref<Texture2D> texture;

        TextureComponent() = default;
        TextureComponent(const TextureComponent&) = default;
        TextureComponent(const Ref<Texture2D>& texture)
            : texture(texture)
        {
        }

    };

    struct OverlayComponent // for now this is used for the heads-up display
    {
        glm::vec4 color = glm::vec4(1.0f); // default white color

        OverlayComponent() = default;
        OverlayComponent(const OverlayComponent&) = default;
        OverlayComponent(const glm::vec4& color)
            : color(color)
        {
        }
    };


    struct CameraComponent
    {
        SceneCamera camera;
        bool primary = false;
        bool hud = false; // heads-up display
        bool fixedAspectRatio = false;

        CameraComponent(const CameraComponent&) = default;
        CameraComponent(ProjectionType projectionType)
            :camera(projectionType)
        {
        }
    };

}
