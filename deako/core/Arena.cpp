#include "deako_pch.h"
#include "Arena.h"


namespace Deako {

	Arena::Arena(uint64_t capacity)
	{
		DK_CORE_ASSERT(capacity > 0, "Arena capacity must be greater than 0!");

		if (capacity % PAGE_SIZE != 0)
		{   // allign to page size
			capacity += PAGE_SIZE - (capacity % PAGE_SIZE);
			DK_CORE_INFO("Arena capacity alligned to page size: {0}", capacity);
		}

		std::byte* memory = static_cast<std::byte*>(Allocate(capacity));

		m_Memory = memory;
		m_Capacity = capacity;
		m_Position = 0;
		m_Nodes = nullptr;
	}

	Arena::~Arena()
	{
		if (m_Memory)
		{
			Release(m_Memory);
		}
	}

	ArenaBlock* Arena::AddBlock(uint64_t size)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		std::uint64_t blockSize = size + sizeof(ArenaNode);

		if (blockSize % PAGE_SIZE != 0)
		{   // allign to page size
			blockSize += PAGE_SIZE - (blockSize % PAGE_SIZE);
		}

		std::uint64_t pageCount = blockSize / PAGE_SIZE;

		blockSize = pageCount * PAGE_SIZE;
		blockSize -= sizeof(ArenaNode);

		if (m_Position + blockSize > m_Capacity)
		{
			return nullptr;
		}

		ArenaNode* node = reinterpret_cast<ArenaNode*>(m_Memory + m_Position);

		std::byte* blockData = m_Memory + sizeof(ArenaNode);

		node->Block = ArenaBlock(blockData, blockSize);
		node->Status = AreaBlockStatus::Active;

		if (m_Nodes)
		{
			node->Next = m_Nodes;
			node->Previous = m_Nodes->Previous;
			m_Nodes->Previous->Next = node;
			m_Nodes->Previous = node;
		}
		else
		{
			m_Nodes = node;
			node->Next = m_Nodes;
			node->Previous = m_Nodes;
		}

		m_Position += size;

		return &node->Block;
	}

	void* Arena::Allocate(uint64_t size)
	{
#ifdef DK_PLATFORM_WINDOWS
		void* memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif DK_PLATFORM_MACOS || DK_PLATFORM_LINUX
		void* memory = malloc(size);
		memset(memory, 0, size);
#endif

		DK_CORE_ASSERT(memory, "Failed to allocate memory!");
		return memory;
	}

	void Arena::Release(void* memory)
	{
#ifdef DK_PLATFORM_WINDOWS
		VirtualFree(memory, 0, MEM_RELEASE);
#elif DK_PLATFORM_MACOS || DK_PLATFORM_LINUX
		free(memory);
#endif
	}

	ArenaBlock::ArenaBlock(std::byte* data, uint64_t size)
		: m_Data(data), m_Size(size), m_Position(0)
	{
	}

}