#pragma once

#include <vulkan/vulkan.h>

namespace Deako
{
	/// VulkanQueue encapsulates a VkQueue
	class VulkanQueue : public Inheritor<Object, VulkanQueue>
	{
	public:
		VulkanQueue(VkQueue queue);
		~VulkanQueue();

		operator VkQueue() const { return m_Queue; }
		VkQueue Vk() const { return m_Queue; }

	private:
		VkQueue m_Queue;
	};

	DK_TYPE_NAME(VulkanQueue);
}

