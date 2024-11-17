#pragma once

#include "Deako/Renderer/EditorCamera.h"

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
            : tag(tag) {
        }
    };

    struct TransformComponent
    {
        DkVec3 translation = { 0.0f, 0.0f, 0.0f };
        DkVec3 rotation = { 0.0f, 0.0f, 0.0f };
        DkVec3 scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;

        DkMat4 GetTransform() const
        {
            return glm::translate(DkMat4(1.0f), translation) *
                glm::toMat4(glm::quat(rotation)) *
                glm::scale(DkMat4(1.0f), scale);
        }

        DkVec3 NormalizeRotation(const DkVec3& rotation)
        {
            static DkF32 MAX_ROTATION = 2 * glm::pi<DkF32>();

            DkVec3 normalizedRotation;
            normalizedRotation.x = fmod(rotation.x, MAX_ROTATION);
            normalizedRotation.y = fmod(rotation.y, MAX_ROTATION);
            normalizedRotation.z = fmod(rotation.z, MAX_ROTATION);

            if (normalizedRotation.x < 0) normalizedRotation.x += MAX_ROTATION;
            if (normalizedRotation.y < 0) normalizedRotation.y += MAX_ROTATION;
            if (normalizedRotation.z < 0) normalizedRotation.z += MAX_ROTATION;

            return normalizedRotation;
        }
    };

    struct TextureComponent
    {
        AssetHandle handle = 0;

        TextureComponent() = default;
        TextureComponent(const TextureComponent&) = default;
    };

    struct MaterialComponent
    {
        AssetHandle handle = 0;

        MaterialComponent() = default;
        MaterialComponent(const MaterialComponent&) = default;
    };

    struct ModelComponent
    {
        AssetHandle handle = 0;

        ModelComponent() = default;
        ModelComponent(AssetHandle handle)
            : handle(handle) {
        }
        ModelComponent(const ModelComponent&) = default;
    };

    struct PrefabComponent
    {
        AssetHandle handle = 0;
        AssetHandle meshHandle = 0;
        std::vector<AssetHandle> textureHandles;
        std::vector<AssetHandle> materialHandles;

        PrefabComponent() = default;
        PrefabComponent(const PrefabComponent&) = default;
        PrefabComponent(AssetHandle handle)
            : handle(handle) {
        }
    };

    struct EnvironmentComponent
    {
        bool active;

        EnvironmentComponent() = default;
        EnvironmentComponent(const EnvironmentComponent&) = default;
    };

    struct CameraComponent
    {
        Ref<EditorCamera> camera;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    template<typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents =
        ComponentGroup<TransformComponent, TextureComponent, MaterialComponent, ModelComponent, PrefabComponent, EnvironmentComponent, CameraComponent>;


}
