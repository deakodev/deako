#pragma once

#include <GLFW/glfw3.h>

namespace Deako {

    struct VulkanSettings
    {
        bool validation = false;
    };

    class VulkanBase
    {
    public:
        static void Init();
        static void Idle();
        static void Shutdown();

        static VkInstance GetInstance() { return s_Instance; }
        static const std::vector<const char*>& GetValidations() { return s_ValidationLayers; };

        static bool ValidationsEnabled() { return s_Settings.validation; }

        static void DrawFrame();

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    private:
        static void CreateInstance();
        static void DetermineExtensions();
        static bool AreValidationsAvailable();

    private:
        static VkInstance s_Instance;
        static std::vector<const char*> s_Extensions;
        static std::vector<const char*> s_ValidationLayers;

        static VkDevice s_Device;
        static VkQueue s_GraphicsQueue;
        static VkQueue s_PresentQueue;

        static uint32_t s_CurrentFrame;
        static bool s_FramebufferResized;

        static std::vector<VkSemaphore> s_ImageAvailableSemaphores;
        static std::vector<VkSemaphore> s_RenderFinishedSemaphores;
        static std::vector<VkFence> s_InFlightFences;

        static VulkanSettings s_Settings;
    };

}
