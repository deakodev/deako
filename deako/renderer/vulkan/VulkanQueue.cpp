#include "deako_pch.h"
#include "VulkanQueue.h"

namespace Deako {

	VulkanQueue::VulkanQueue(VkQueue queue)
		: m_Queue(queue)
	{
	}

	VulkanQueue::~VulkanQueue()
	{
	}

}

