#pragma once

#include "Deako/Core/UUID.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

    struct IDComponent
    {
        UUID id;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

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
        std::vector<AssetHandle> handles;

        TextureComponent() = default;
        TextureComponent(const TextureComponent&) = default;
    };

    struct MaterialComponent
    {
        std::vector<AssetHandle> handles;

        MaterialComponent() = default;
        MaterialComponent(const MaterialComponent&) = default;
    };

    struct ModelComponent
    {
        AssetHandle handle;

        ModelComponent() = default;
        ModelComponent(AssetHandle handle)
            : handle(handle) {}
        ModelComponent(const ModelComponent&) = default;
    };

    struct PrefabComponent
    {
        AssetHandle handle;

        PrefabComponent() = default;
        PrefabComponent(const PrefabComponent&) = default;
    };

    struct EnvironmentComponent
    {
        bool active;

        EnvironmentComponent() = default;
        EnvironmentComponent(const EnvironmentComponent&) = default;
    };

}
