#pragma once

#include "Deak/Renderer/Texture.h"

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

}
