#include "ScenePanel.h"

#include "Deako/Scene/Components.h"
#include "Deako/Asset/AssetPool.h"

#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    ScenePanel::ScenePanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void ScenePanel::SetContext(const Ref<Scene>& context)
    {
        m_Context = context;
        m_SelectionContext = {};
    }

    void ScenePanel::OnImGuiRender()
    {
        ImGui::Begin("Scene");

        auto view = m_Context->m_Registry.view<TagComponent>();
        for (auto entityHandle : view)
        {
            Entity entity{ entityHandle, m_Context.get() };
            DrawEntityNode(entity);
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_SelectionContext = {};

        // Popup Menu - after right click on blank space
        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_Context->CreateEntity("Empty Entity");

            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Begin("Properties");

        if (m_SelectionContext)
            DrawComponents(m_SelectionContext);

        ImGui::End();
    }

    void ScenePanel::DrawEntityNode(Entity entity)
    {
        ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0)
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        auto& tag = entity.GetComponent<TagComponent>().tag;
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());

        if (ImGui::IsItemClicked()) m_SelectionContext = entity;

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
            m_Context->DestroyEntity(entity); // TODO
            if (m_SelectionContext == entity)
                m_SelectionContext = {};
        }
    }

    void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 90.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 4, 4 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y + 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);

        ImGui::PopID();
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
            | ImGuiTreeNodeFlags_AllowItemOverlap
            | ImGuiTreeNodeFlags_Framed
            | ImGuiTreeNodeFlags_FramePadding
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();

            ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
            ImGui::PopStyleVar();
            ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
            if (ImGui::Button(Icons::Trash.c_str(), ImVec2{ lineHeight, lineHeight }))
            {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem("Remove Component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveComponent<T>();
        }
    }

    void ScenePanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().tag;

            char buffer[256];
            std::memset(buffer, 0, sizeof(buffer));
            std::strncpy(buffer, tag.c_str(), sizeof(buffer) - 1);
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                tag = std::string(buffer);
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            if (ImGui::MenuItem("Color"))
            {
                if (!m_SelectionContext.HasComponent<TextureComponent>())
                    m_SelectionContext.AddComponent<TextureComponent>();
                else
                    DK_CORE_WARN("This entity already has TextureComponent!");
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Sprite Renderer"))
            {
                if (!m_SelectionContext.HasComponent<ModelComponent>())
                    m_SelectionContext.AddComponent<ModelComponent>();
                else
                    DK_CORE_WARN("This entity already has ModelComponent!");
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }


        ImGui::PopItemWidth();

        DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
            {
                DrawVec3Control("Translation", component.translation);

                glm::vec3 rotation = glm::degrees(component.rotation);
                DrawVec3Control("Rotation", rotation);
                component.rotation = glm::radians(rotation);

                DrawVec3Control("Scale", component.scale, 1.0f);
            });

        DrawComponent<TextureComponent>("Texture", entity, [](auto& component)
            {
                ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
            });

        DrawComponent<MaterialComponent>("Material", entity, [](auto& component)
            {
                Ref<Material> material = AssetPool::GetAsset<Material>(component.handles[1]);

                if (material)
                {
                    // Name
                    char buffer[256];
                    std::memset(buffer, 0, sizeof(buffer));
                    std::strncpy(buffer, material->name.c_str(), sizeof(buffer) - 1);

                    if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
                        material->name = std::string(buffer);

                    ImGui::Text("Base Color");
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##BaseColor", (float*)&material->baseColorFactor, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_NoLabel);

                    if (ImGui::IsItemDeactivatedAfterEdit()) // detect when user stops dragging
                    {
                        AssetPool::Invalidate(material);
                    }

                    // metallicFactor
                    static float minFactor = 0.0f;
                    static float maxFactor = 1.0f;

                    ImGui::Text("Metallic Factor");
                    ImGui::SameLine();
                    ImGui::DragScalar("##MetallicFactor", ImGuiDataType_Float, &material->metallicFactor, 0.005f, &minFactor, &maxFactor, "%.3f");

                    if (ImGui::IsItemDeactivatedAfterEdit()) // detect when user stops dragging
                    {
                        AssetPool::Invalidate(material);
                    }

                    ImGui::Text("Roughness Factor");
                    ImGui::SameLine();
                    ImGui::DragScalar("##RoughnessFactor", ImGuiDataType_Float, &material->roughnessFactor, 0.005f, &minFactor, &maxFactor, "%.3f");

                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        AssetPool::Invalidate(material);
                    }
                }
            });

        DrawComponent<ModelComponent>("Model", entity, [](auto& component)
            {
                ImGui::Button("Model", ImVec2(100.0f, 0.0f));
            });

    }

}
