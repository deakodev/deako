#pragma once

#include "Deako.h"

namespace Deako {

    template<typename T>
    struct Context
    {
        Ref<T> context;
        bool isValid{ false };

        void Set(Ref<T> newContext)
        {
            if (newContext)
            {
                context = newContext;
                isValid = true;
            }
        }

        void Reset()
        {
            context = nullptr;
            isValid = false;
        }

        Ref<T> operator->() { return context; }
        bool operator==(Ref<T> other) { return context == other; }
        explicit operator bool() const { return isValid; }
    };

    struct EditorContext
    {
        Context<Entity> entity;
        Context<Scene> scene;
        Context<ProjectAssetPool> assetPool;
        Context<Project> project;

        EditorContext();

        void OnUpdate();
    };

}
