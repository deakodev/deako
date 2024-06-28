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
        static void CleanUp();

        static VkInstance GetInstance() { return s_Instance; }

    private:
        static void CreateInstance();
        static void DetermineExtensions();
        static bool AreValidationsAvailable();

    private:
        static VkInstance s_Instance;
        static std::vector<const char*> s_Extensions;
        static std::vector<const char*> s_ValidationLayers;

        static VulkanSettings s_Settings;
    };

}
