#pragma once

namespace Deako {

	// non-owning raw buffer class
	struct Buffer
	{
		uint8_t* data{ nullptr };
		uint64_t size{ 0 };

		Buffer() = default;

		Buffer(uint64_t bufferSize)
		{
		}

		Buffer(const void* data, uint64_t bufferSize)
			:data((uint8_t*)data), size(bufferSize)
		{
		}

		Buffer(const Buffer&) = default;

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.size);
			memcpy(result.data, other.data, other.size);
			return result;
		}

		void Allocate(uint64_t bufferSize)
		{
			Release();

			data = (uint8_t*)malloc(bufferSize);
			size = bufferSize;
		}

		void Release()
		{
			free(data);
			data = nullptr;
			size = 0;
		}

		template<typename T>
		T* As()
		{
			return (T*)data;
		}

		operator bool() const
		{
			return (bool)data;
		}

	};

}
