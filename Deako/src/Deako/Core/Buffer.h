#pragma once

#include "Deako/Math/Math.h"

namespace Deako {

	// non-owning raw buffer class
	struct Buffer
	{
		DkU8* data{ nullptr };
		DkU64 size{ 0 };

		Buffer() = default;

		Buffer(DkU64 bufferSize)
		{
		}

		Buffer(const void* data, DkU64 bufferSize)
			:data((DkU8*)data), size(bufferSize)
		{
		}

		Buffer(const Buffer&) = default;

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.size);
			memcpy(result.data, other.data, other.size);
			return result;
		}

		void Allocate(DkU64 bufferSize)
		{
			Release();

			data = (DkU8*)malloc(bufferSize);
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
