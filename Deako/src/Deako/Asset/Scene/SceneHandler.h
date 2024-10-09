#pragma once

#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h"
#include "Deako/Renderer/Renderer.h"

namespace Deako {

    enum class UserResponse;

    class SceneHandler
    {
    public:
        static void Init();
        static void CleanUp();

        static void NewScene();
        static void OpenScene();
        static void SaveScene();
        static void SaveAsScene();

        static void ImportScene(std::filesystem::path path);
        static Ref<Scene> ImportScene(AssetHandle handle, AssetMetadata& metadata);

        static void SetActiveScene(AssetHandle handle);
        static void SetActiveScene(Ref<Scene> scene);
        static Ref<Scene> GetActiveScene() { return s_ActiveScene; }
        static Ref<Scene> GetEmptyScene() { return s_EmptyScene; }

        static void RefreshScene();

    private:
        static void SaveScene(AssetMetadata& metadata);
        static void PromptToOpenScene();
        static bool PromptToSaveScene();

        static bool HandleUserResponse(UserResponse response);

    private:
        inline static Ref<Scene> s_ActiveScene;
        inline static Ref<Scene> s_EmptyScene;
    };



}
