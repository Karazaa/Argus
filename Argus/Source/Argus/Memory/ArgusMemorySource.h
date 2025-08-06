// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

/*
 *
 * Arena to be used for all of the dynamic allocations in the gameplay logic of Argus. I can't control what the engine does with its allocations¯\_(ツ)_/¯
 *
 */

class ArgusMemorySource
{
	static char* s_rawDataRoot;
	static SIZE_T s_capacity;
	static SIZE_T s_occupiedAmount;
	static SIZE_T s_totalLossAmount;

public:
	static constexpr SIZE_T k_1MB = 1048576;
	static constexpr SIZE_T k_10MB = 10485760;
	static constexpr SIZE_T k_100MB = 104857600;

	static void Initialize(SIZE_T memorySourceSize = k_100MB, uint32 alignment = DEFAULT_ALIGNMENT);
	static void ResetMemorySource();
	static void TearDown();

	static void* Allocate(SIZE_T allocationSize, uint32 alignment = DEFAULT_ALIGNMENT);
	template<typename T>
	static T* Allocate();

	static void* Reallocate(void* oldData, SIZE_T bytesPerElement, SIZE_T oldNumElements, SIZE_T newNumElements, uint32 alignment = DEFAULT_ALIGNMENT);
	template<typename T>
	static T* Reallocate(T* oldData, SIZE_T oldNumElements, SIZE_T newNumElements, uint32 alignment = DEFAULT_ALIGNMENT);

	static void Deallocate(void* data);
	static void Deallocate(void* data, SIZE_T allocationSize);
	template<typename T>
	static void Deallocate(T* data);

	static void CopyMemory(void* destination, void* source, SIZE_T amount);

	static SIZE_T GetCapacity() { return s_capacity; }
	static SIZE_T GetOccupiedAmount() { return s_occupiedAmount; }
	static SIZE_T GetTotalLossAmount() { return s_totalLossAmount; }
	static SIZE_T GetAvailableSpace() { return s_capacity - s_occupiedAmount; }
};

template <typename T>
T* ArgusMemorySource::Allocate()
{
	void* allocatedMemory = Allocate(sizeof(T), alignof(T));
	if (!allocatedMemory)
	{
		return nullptr;
	}

	return new (allocatedMemory) T();
}

template <typename T>
T* ArgusMemorySource::Reallocate(T* oldData, SIZE_T oldNumElements, SIZE_T newNumElements, uint32 alignment)
{
	return static_cast<T*>(Reallocate(oldData, sizeof(T), oldNumElements, newNumElements, alignment));
}

template <typename T>
void ArgusMemorySource::Deallocate(T* data)
{
	Deallocate(data, sizeof(T));
}