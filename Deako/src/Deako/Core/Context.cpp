#include "Context.h"
#include "dkpch.h"

#include "Deako/Renderer/Vulkan/VulkanPicker.h"

// temp
#include <ImGuizmo.h>

namespace Deako {

    static DkContext* Gdeako = nullptr;

    DkContext& CreateContext(DkContextFlags flags)
    {
        Gdeako = new DkContext();
        DkContext& deako = *Gdeako;

        DK_CORE_ASSERT(!deako.initialized, "DkContext already initialized!");

        deako.configFlags = flags;

        deako.window = ConfigureWindow(Gdeako); // defined/configured on the client side
        deako.application = ConfigureApplication(Gdeako); // defined/configured on the client side
        deako.input = CreateScope<DkInput>();
        deako.activeProject = CreateScope<Project>();
        deako.projectAssetPool = CreateScope<ProjectAssetPool>();
        deako.editorAssetPool = CreateScope<EditorAssetPool>();

        if (flags & DkContextFlags_ImplVulkan)
            Renderer::Init();

        deako.initialized = true;

        return deako;
    }

    void DestroyContext()
    {
        DkContext& deako = *Gdeako;

        if (deako.configFlags & DkContextFlags_ImplVulkan)
            Renderer::Shutdown();

        delete Gdeako;
        Gdeako = nullptr;
    }

    DkContext& GetContext()
    {
        return *Gdeako;
    }

    DkApplication& GetApplication()
    {
        return *Gdeako->application;
    }

    DkWindow& GetWindow()
    {
        return *Gdeako->window;
    }

    DkInput& GetInput()
    {
        return *Gdeako->input;
    }

    Scene& GetActiveScene()
    {
        DK_CORE_ASSERT(Gdeako->activeSceneHandle != 0, "No active scene!");
        return *Gdeako->projectAssetPool->GetAsset<Scene>(Gdeako->activeSceneHandle);
    }

    void SetActiveScene(DkHandle handle)
    {
        Ref<Scene> scene = Gdeako->projectAssetPool->GetAsset<Scene>(handle);
        SetActiveScene(scene);
    }

    void SetActiveScene(Ref<Scene> scene)
    {
        Gdeako->activeSceneHandle = scene ? scene->m_Handle : s_EmptyScene->m_Handle;

        DK_CORE_ASSERT(scene != 0, "No active scene!");
        scene->LinkAssets();
    }

    void SetHoveredHandle(DkHandle handle)
    {
        Gdeako->hoveredHandle = handle;
    }

    ProjectAssetPool& GetProjectAssetPool()
    {
        return *Gdeako->projectAssetPool;
    }

    void NewFrame()
    {
        DkContext& deako = *Gdeako;

        deako.hoveredHandlePreviousFrame = deako.hoveredHandle;

        const DkVec4& pickerColor = VulkanPicker::GetPixelColor();
        deako.hoveredHandle = GetActiveScene().GetEntityHandle(pickerColor);

        deako.activeHandlePreviousFrame = deako.activeHandle;

        if (IsMousePressed(Mouse::ButtonLeft) && !IsKeyPressed(Key::LeftAlt) && !AreEventsBlocked() && !ImGuizmo::IsOver())
        {
            deako.activeHandle = deako.hoveredHandle;
        }

        GetInput().OnUpdate();

        GetActiveScene().OnUpdate();
    }

    void Render()
    {
        Renderer::Render();
    }

    const RendererStats& GetSceneStats()
    {
        return Renderer::GetSceneStats();
    }


}
