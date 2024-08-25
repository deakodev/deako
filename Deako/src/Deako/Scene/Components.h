#pragma once

#include "Deako/Renderer/Vulkan/VulkanModel.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

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

        // glm::mat4 GetTransform() const
        // {
        //     glm::mat4 _rotation = glm::toMat4(glm::quat(rotation));

        //     return glm::translate(glm::mat4(1.0f), translation)
        //         * _rotation * glm::scale(glm::mat4(1.0f), scale);
        // }
    };

    struct TextureComponent
    {
        AssetHandle texture;

        TextureComponent() = default;
        TextureComponent(const TextureComponent&) = default;
        TextureComponent(AssetHandle texture)
            : texture(texture) {}
    };

    struct ModelComponent
    {
        enum Usage { NONE = 0, PROP = 1, ENVIRONMENT = 2 };
        Usage usage{ NONE };

        AssetHandle model;

        ModelComponent() = default;
        ModelComponent(const ModelComponent&) = default;
    };






}
