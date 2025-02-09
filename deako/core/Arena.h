#pragma once

namespace Deako {

	constexpr uint64_t ARENA_CAPACITY = 1024 * 1024 * 1024; // 1GB
	constexpr uint64_t PAGE_SIZE = 1024; // 1KB

	enum class AreaBlockStatus : uint8_t
	{
		Active = 0,
		Free = 1,
	};

	class ArenaBlock
	{
	public:

		std::uint64_t RemainingSpace() const noexcept { return m_Size - m_Position; }

		template<typename Object>
		void Push(const Object& object)
		{
			*reinterpret_cast<Object*>(m_Data + m_Position) = object;
			m_Position += sizeof(object);
		}

	private:
		ArenaBlock(std::byte* data, uint64_t size);

	private:
		std::byte* m_Data;
		uint64_t m_Size;
		uint64_t m_Position;

		friend class Arena;
	};

	struct ArenaNode
	{
		ArenaBlock Block;
		AreaBlockStatus Status;

		ArenaNode* Next;
		ArenaNode* Previous;
	};

	class Arena
	{
	public:
		Arena(uint64_t capacity = ARENA_CAPACITY);
		~Arena();

		Arena(const Arena&) = delete;
		Arena& operator=(const Arena&) = delete;

		ArenaBlock* AddBlock(uint64_t size);

		std::uint64_t RemainingSpace() const noexcept { return m_Capacity - m_Position; }

		template<typename Object>
		void Push(const Object& object)
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			if (m_Nodes)
			{
				m_Nodes->Block.Push(object);
			}
			else
			{
				ArenaBlock* block = AddBlock(sizeof(Object));
				block->Push(object);
			}
		}

	private:
		static void* Allocate(uint64_t size);
		static void Release(void* memory);

	private:
		std::byte* m_Memory;
		uint64_t m_Capacity;
		uint64_t m_Position;

		ArenaNode* m_Nodes;

		mutable std::mutex m_Mutex;
	};

}

