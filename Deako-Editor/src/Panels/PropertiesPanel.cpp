#include "PropertiesPanel.h"

#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    void PropertiesPanel::OnImGuiRender()
    {
        DkContext& deako = Deako::GetContext();
        ProjectAssetPool& projectAssetPool = Deako::GetProjectAssetPool();
        Scene& activeScene = Deako::GetActiveScene();

        ImGui::Begin("Properties");

        bool eventsBlocked = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) || ImGui::IsAnyItemHovered();
        Deako::BlockEvents(eventsBlocked);

        if (deako.activeHandle != 0)
        {
            const Entity& entity = activeScene.GetEntity(deako.activeHandle);
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
                    if (!entity.HasComponent<TextureComponent>())
                        entity.AddComponent<TextureComponent>();
                    else
                        DK_CORE_WARN("This entity already has TextureComponent!");
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Sprite Renderer"))
                {
                    if (!entity.HasComponent<ModelComponent>())
                        entity.AddComponent<ModelComponent>();
                    else
                        DK_CORE_WARN("This entity already has ModelComponent!");
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::PopItemWidth();

            DrawComponents(entity, projectAssetPool);
        }

        ImGui::End();
    }

    void DrawVec3Control(const std::string& label, DkVec3& values, DkF32 resetValue = 0.0f, DkF32 columnWidth = 90.0f)
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

        DkF32 lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y + 2.0f;
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
    void PropertiesPanel::DrawComponent(const std::string& name, const Entity& entity, ProjectAssetPool& assetPool, UIFunction uiFunction)
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
            DkF32 lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

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

    void PropertiesPanel::DrawComponents(const Entity& entity, ProjectAssetPool& assetPool)
    {
        DrawComponent<TransformComponent>("Transform", entity, assetPool, [](auto& component)
            {
                DrawVec3Control("Translation", component.translation);

                DkVec3 rotation = glm::degrees(component.rotation);
                DrawVec3Control("Rotation", rotation);
                component.rotation = glm::radians(rotation);

                DrawVec3Control("Scale", component.scale, 1.0f);
            });

        DrawComponent<TextureComponent>("Texture", entity, assetPool, [&assetPool](auto& component)
            {
                bool isTextureValid = false;
                std::string label = "None";
                if (component.handle != 0)
                {
                    const AssetMetadata& metadata = assetPool.GetAssetMetadata(component.handle);
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
                DkF32 buttonWidth = glm::max<DkF32>(100.0f, buttonSize.x);

                ImGui::Button(label.c_str(), ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
                    {
                        AssetHandle newHandle = *(AssetHandle*)payload->Data;

                        AssetType newAssetType = assetPool.GetAssetType(newHandle);
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
                    DkF32 lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
                    if (ImGui::Button("Delete", ImVec2{ lineHeight, lineHeight }))
                    {
                        component.handle = 0;
                    }
                }
            });

        DrawComponent<MaterialComponent>("Material", entity, assetPool, [&assetPool](auto& component)
            {
                Ref<Material> material = assetPool.GetAsset<Material>(component.handle);

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
                    ImGui::ColorEdit4("##BaseColor", (DkF32*)&material->baseColorFactor, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_NoLabel);

                    if (ImGui::IsItemDeactivatedAfterEdit()) // detect when user stops dragging
                        SceneHandler::RefreshScene();

                    // metallicFactor
                    static DkF32 minFactor = 0.0f;
                    static DkF32 maxFactor = 1.0f;

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

        DrawComponent<ModelComponent>("Model", entity, assetPool, [](auto& component)
            {
                ImGui::Button("Model", ImVec2(100.0f, 0.0f));
            });

    }

}
