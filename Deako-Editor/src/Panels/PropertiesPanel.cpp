#include "PropertiesPanel.h"

#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    PropertiesPanel::PropertiesPanel(Ref<EditorContext> editorContext)
        : m_EditorContext(editorContext)
    {
    }

    void PropertiesPanel::OnImGuiRender()
    {
        ImGui::Begin("Properties");

        if (m_EditorContext->entity)
        {
            if (m_EditorContext->entity->HasComponent<TagComponent>())
            {
                auto& tag = m_EditorContext->entity->GetComponent<TagComponent>().tag;

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
                    if (!m_EditorContext->entity->HasComponent<TextureComponent>())
                        m_EditorContext->entity->AddComponent<TextureComponent>();
                    else
                        DK_CORE_WARN("This entity already has TextureComponent!");
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Sprite Renderer"))
                {
                    if (!m_EditorContext->entity->HasComponent<ModelComponent>())
                        m_EditorContext->entity->AddComponent<ModelComponent>();
                    else
                        DK_CORE_WARN("This entity already has ModelComponent!");
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::PopItemWidth();

            DrawComponents();
        }

        ImGui::End();
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
    void PropertiesPanel::DrawComponent(const std::string& name, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
            | ImGuiTreeNodeFlags_AllowItemOverlap
            | ImGuiTreeNodeFlags_Framed
            | ImGuiTreeNodeFlags_FramePadding
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (m_EditorContext->entity->HasComponent<T>())
        {
            auto& component = m_EditorContext->entity->GetComponent<T>();

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
                m_EditorContext->entity->RemoveComponent<T>();
        }
    }

    void PropertiesPanel::DrawComponents()
    {
        DrawComponent<TransformComponent>("Transform", [](auto& component)
            {
                DrawVec3Control("Translation", component.translation);

                glm::vec3 rotation = glm::degrees(component.rotation);
                DrawVec3Control("Rotation", rotation);
                component.rotation = glm::radians(rotation);

                DrawVec3Control("Scale", component.scale, 1.0f);
            });

        DrawComponent<TextureComponent>("Texture", [this](auto& component)
            {
                bool isTextureValid = false;
                std::string label = "None";
                if (component.handle != 0)
                {
                    const AssetMetadata& metadata = m_EditorContext->assetPool->GetAssetMetadata(component.handle);
                    if (metadata.assetType == AssetType::Texture2D)
                    {
                        label = metadata.assetPath.filename().string() + " (Texture2D)";
                        isTextureValid = true;
                    }
                    else if (metadata.assetType == AssetType::TextureCubeMap)
                    {
                        label = metadata.assetPath.filename().string() + " (TextureCubeMap)";
                        isTextureValid = true;
                    }
                    else
                    {
                        label = "Empty";
                    }
                }

                ImVec2 buttonSize = ImGui::CalcTextSize(label.c_str());
                buttonSize.x += 20.0f; // padding
                float buttonWidth = glm::max<float>(100.0f, buttonSize.x);

                ImGui::Button(label.c_str(), ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
                    {
                        AssetHandle newHandle = *(AssetHandle*)payload->Data;

                        AssetType newAssetType = m_EditorContext->assetPool->GetAssetType(newHandle);
                        if (newAssetType == AssetType::Texture2D || newAssetType == AssetType::TextureCubeMap)
                        {
                            component.handle = newHandle;
                            SceneHandler::RefreshScene();
                        }
                        else
                        {
                            DK_WARN("Wrong asset type!");
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                if (isTextureValid)
                {
                    ImGui::SameLine(0.0f, 10.0f);
                    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
                    if (ImGui::Button("Delete", ImVec2{ lineHeight, lineHeight }))
                    {
                        component.handle = 0;
                    }
                }
            });

        DrawComponent<MaterialComponent>("Material", [this](auto& component)
            {
                Ref<Material> material = m_EditorContext->assetPool->GetAsset<Material>(component.handle);

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
                        SceneHandler::RefreshScene();

                    // metallicFactor
                    static float minFactor = 0.0f;
                    static float maxFactor = 1.0f;

                    ImGui::Text("Metallic Factor");
                    ImGui::SameLine();
                    ImGui::DragScalar("##MetallicFactor", ImGuiDataType_Float, &material->metallicFactor, 0.005f, &minFactor, &maxFactor, "%.3f");

                    if (ImGui::IsItemDeactivatedAfterEdit()) // detect when user stops dragging
                        SceneHandler::RefreshScene();

                    ImGui::Text("Roughness Factor");
                    ImGui::SameLine();
                    ImGui::DragScalar("##RoughnessFactor", ImGuiDataType_Float, &material->roughnessFactor, 0.005f, &minFactor, &maxFactor, "%.3f");

                    if (ImGui::IsItemDeactivatedAfterEdit())
                        SceneHandler::RefreshScene();
                }
            });

        DrawComponent<ModelComponent>("Model", [](auto& component)
            {
                ImGui::Button("Model", ImVec2(100.0f, 0.0f));
            });

    }

}
