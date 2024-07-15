#include "VulkanSync.h"
#include "dkpch.h"

namespace Deako {

    Ref<VulkanResources> Sync::s_VR = VulkanBase::GetResources();
    Ref<VulkanSettings> Sync::s_VS = VulkanBase::GetSettings();

    void Sync::CreateObjects()
    {
        s_VR->imageAvailableSemaphores.resize(s_VS->imageCount);
        s_VR->renderFinishedSemaphores.resize(s_VS->imageCount);
        s_VR->inFlightFences.resize(s_VS->imageCount);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // so first call to vkWaitForFences() returns immediately since the fence is already signaled

        for (size_t i = 0; i < s_VS->imageCount; i++)
        {
            VkResult result = vkCreateSemaphore(s_VR->device, &semaphoreInfo, nullptr, &s_VR->imageAvailableSemaphores[i]);
            DK_CORE_ASSERT(!result);

            result = vkCreateSemaphore(s_VR->device, &semaphoreInfo, nullptr, &s_VR->renderFinishedSemaphores[i]);
            DK_CORE_ASSERT(!result);

            result = vkCreateFence(s_VR->device, &fenceInfo, nullptr, &s_VR->inFlightFences[i]);
            DK_CORE_ASSERT(!result);
        }
    }

    void Sync::CleanUp()
    {
        for (size_t i = 0; i < s_VS->imageCount; i++)
        {
            vkDestroySemaphore(s_VR->device, s_VR->renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(s_VR->device, s_VR->imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(s_VR->device, s_VR->inFlightFences[i], nullptr);
        }
    }

}
