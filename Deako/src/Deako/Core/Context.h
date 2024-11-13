#pragma once

#include "Deako/Core/Application.h"
#include "Deako/Core/Window.h"
#include "Deako/Core/Input.h"
#include "Deako/Core/Handle.h"
#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Project/Project.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Pool/EditorAssetPool.h"
#include "Deako/Renderer/Renderer.h"

namespace Deako {

    typedef int DkContextFlags;         // -> enum DkContextFlags_

    enum DkContextFlags_
    {
        DkContextFlags_None = 0,
        DkContextFlags_EditorRuntime = 1 << 0,   // 1
        DkContextFlags_GameRuntime = 1 << 1,     // 2
        DkContextFlags_ImplVulkan = 1 << 2       // 4
    };

    struct DkContext
    {
        DkContextFlags configFlags;

        Scope<DkWindow> window;
        Scope<DkApplication> application;
        Scope<DkInput> input;

        DkHandle hoveredHandle = 0;
        DkHandle hoveredHandlePreviousFrame = 0;
        DkHandle activeHandle = 0;
        DkHandle activeHandlePreviousFrame = 0;

        DkHandle activeSceneHandle = 0;

        Scope<Project> activeProject;
        Scope<ProjectAssetPool> projectAssetPool;
        Scope<EditorAssetPool> editorAssetPool;

        bool initialized;
    };

    DkContext& CreateContext(DkContextFlags flags = DkContextFlags_EditorRuntime | DkContextFlags_ImplVulkan);

    void DestroyContext();

    DkContext& GetContext();
    DkApplication& GetApplication();
    DkWindow& GetWindow();
    DkInput& GetInput();
    Scene& GetActiveScene();
    ProjectAssetPool& GetProjectAssetPool();

    void SetActiveScene(DkHandle handle);
    void SetActiveScene(Ref<Scene> scene);
    void SetHoveredHandle(DkHandle handle);

    void NewFrame();
    void Render();

}
