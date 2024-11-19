#include "ScenePanel.h"

#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    void ScenePanel::OnImGuiRender()
    {
        DkContext& deako = Deako::GetContext();
        Scene& activeScene = Deako::GetActiveScene();

        ImGui::Begin("Scene");

        for (auto& entity : activeScene.GetEntities())
        {
            EntityHandle entityHandle = entity.GetHandle();
            DrawEntityNode(entityHandle, deako.activeHandle == entityHandle);
        }

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            deako.activeHandle = 0;
        }

        // Popup Menu - after right click on blank space
        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                Scene& activeScene = Deako::GetActiveScene();
                activeScene.CreateEntity("Empty Entity");
                activeScene.Invalidate();
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void ScenePanel::DrawEntityNode(EntityHandle entityHandle, bool isNodeSelected)
    {
        DkContext& deako = Deako::GetContext();

        ImGuiTreeNodeFlags flags = (isNodeSelected ? ImGuiTreeNodeFlags_Selected : 0)
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        std::string& tag = Entity::GetComponent<TagComponent>(entityHandle).tag;

        bool opened = ImGui::TreeNodeEx((void*)(uintptr_t)entityHandle, flags, "%s", tag.c_str());

        if (ImGui::IsItemClicked())
            deako.activeHandle = entityHandle;

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened) ImGui::TreePop();

        if (entityDeleted)
        {
            // deako.activeEntityHandle->DestroyEntity(entityHandle); // TODO
            // if (deako.activeEntityHandle == entityHandle)
            //     deako.activeEntityHandle = 0;
        }
    }

}
