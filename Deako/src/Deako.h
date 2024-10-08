#pragma once

// This header to be included by Deako apps

#include "Deako/Core/Application.h"

#include "Deako/Core/Log.h"
#include "Deako/Core/Layer.h"
#include "Deako/Core/RateLimiter.h"
// #include "Deako/Core/Input.h"

#include "Deako/Event/KeyCodes.h"
#include "Deako/Event/MouseCodes.h"

#include "Deako/ImGui/ImGuiIcons.h"

#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"

#include "Deako/Asset/Scene/Entity.h"
#include "Deako/Asset/Scene/Components.h"
#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Asset/Scene/SceneHandler.h"
#include "Deako/Asset/Prefab/PrefabHandler.h"
#include "Deako/Asset/Texture/TextureHandler.h"

#include "Deako/Renderer/Renderer.h"
#include "Deako/Renderer/EditorCamera.h"
#include "Deako/Renderer/Vulkan/VulkanScene.h" // TODO: temp

#include "Deako/Project/Project.h"
#include "Deako/Project/ProjectHandler.h"



