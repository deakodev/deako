#include "ScenePanel.h"

#include "../EditorLayer.h"
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    ScenePanel::ScenePanel(Ref<EditorContext> editorContext)
        : m_EditorContext(editorContext)
    {
    }

    void ScenePanel::OnImGuiRender()
    {
        ImGui::Begin("Scene");

        auto view = m_EditorContext->scene->registry.view<TagComponent>();
        bool clickedOnEntity = false;

        for (auto entityHandle : view)
        {
            Ref<Entity> entity = CreateRef<Entity>(entityHandle, m_EditorContext->scene.context.get());
            clickedOnEntity |= DrawEntityNode(entity);
        }

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && !clickedOnEntity)
            m_EditorContext->entity.Reset();

        // Popup Menu - after right click on blank space
        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_EditorContext->scene->CreateEntity("Empty Entity");

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    bool ScenePanel::DrawEntityNode(Ref<Entity> entity)
    {
        bool isNodeSelected = m_EditorContext->entity == entity;

        ImGuiTreeNodeFlags flags = (isNodeSelected ? ImGuiTreeNodeFlags_Selected : 0)
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        auto& tag = entity->GetComponent<TagComponent>().tag;
        bool opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity.get(), flags, "%s", tag.c_str());

        bool entityClicked = false;

        if (ImGui::IsItemClicked())
        {
            m_EditorContext->entity.Set(entity);
            entityClicked = true;
        }

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
            m_EditorContext->scene->DestroyEntity(*entity); // TODO
            if (m_EditorContext->entity == entity)
                m_EditorContext->entity.Reset();
        }

        return entityClicked;
    }

}
