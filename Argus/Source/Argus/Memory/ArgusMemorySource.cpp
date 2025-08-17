// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusMemorySource.h"
#include "ArgusLogging.h"

char* ArgusMemorySource::s_rawDataRoot = nullptr;
SIZE_T ArgusMemorySource::s_capacity = 0;
SIZE_T ArgusMemorySource::s_occupiedAmount = 0;
SIZE_T ArgusMemorySource::s_alignmentLossAmount = 0;
SIZE_T ArgusMemorySource::s_reallocationLossAmount = 0;
SIZE_T ArgusMemorySource::s_deallocationLossAmount = 0;

void ArgusMemorySource::Initialize(SIZE_T memorySourceSize, uint32 alignment)
{
	if (UNLIKELY(s_rawDataRoot))
	{
		ARGUS_LOG(ArgusMemoryLog, Error, TEXT("[%s] Trying to initialize memory source after it has already been initialized!"), ARGUS_FUNCNAME);
		return;
	}

	s_rawDataRoot = static_cast<char*>(FMemory::Malloc(memorySourceSize, alignment));
	s_capacity = memorySourceSize;
	s_occupiedAmount = 0;
	s_alignmentLossAmount = 0;
	s_reallocationLossAmount = 0;
	s_deallocationLossAmount = 0;
}

void ArgusMemorySource::ResetMemorySource()
{
	if (s_rawDataRoot)
	{
		s_occupiedAmount = 0;
		s_alignmentLossAmount = 0;
		s_reallocationLossAmount = 0;
		s_deallocationLossAmount = 0;
	}
	else
	{
		Initialize();
	}
}

void ArgusMemorySource::TearDown()
{
	if (UNLIKELY(!s_rawDataRoot))
	{
		ARGUS_LOG(ArgusMemoryLog, Error, TEXT("[%s] Trying to tear down memory source after it has either already been torn down, or never initialized!"), ARGUS_FUNCNAME);
		return;
	}

	FMemory::Free(s_rawDataRoot);
	s_rawDataRoot = nullptr;
	s_capacity = 0;
	s_occupiedAmount = 0;
	s_alignmentLossAmount = 0;
	s_reallocationLossAmount = 0;
	s_deallocationLossAmount = 0;
}

void* ArgusMemorySource::Allocate(SIZE_T allocationSize, uint32 alignment)
{
	if (UNLIKELY(!s_rawDataRoot))
	{
		Initialize();
	}

	SIZE_T headOfNewData = s_occupiedAmount;
	if (alignment != 0u && !IsAligned(s_occupiedAmount, alignment))
	{
		headOfNewData = Align(s_occupiedAmount, alignment);
	}
	
	if (UNLIKELY(headOfNewData + allocationSize > GetAvailableSpace()))
	{
		ARGUS_LOG(ArgusMemoryLog, Error, TEXT("[%s] Memory source ran out of space!"), ARGUS_FUNCNAME);
		return nullptr;
	}

	const SIZE_T paddingAmount = headOfNewData - s_occupiedAmount;
	s_occupiedAmount += paddingAmount + allocationSize;
	s_alignmentLossAmount += paddingAmount;

	return &s_rawDataRoot[headOfNewData];
}

void* ArgusMemorySource::Reallocate(void* oldData, SIZE_T bytesPerElement, SIZE_T oldNumElements, SIZE_T newNumElements, uint32 alignment)
{
	if (oldData)
	{
		// TODO JAMES: Might be worth logging something about # of frees.
		s_reallocationLossAmount += (bytesPerElement * oldNumElements);
	}

	void* newData = Allocate(bytesPerElement * newNumElements, alignment);

	if (oldNumElements != 0 && newData)
	{
		const SIZE_T memoryCopyAmount = FGenericPlatformMath::Min(oldNumElements, newNumElements);
		CopyMemory(newData, oldData, memoryCopyAmount * bytesPerElement);
	}

	return newData;
}

void ArgusMemorySource::Deallocate(void* data)
{
	// TODO JAMES: Might be worth logging something about # of frees.
	// An uknown amount of memory is now effectively lost. Sadge.
}

void ArgusMemorySource::Deallocate(void* data, SIZE_T allocationSize)
{
	// TODO JAMES: Might be worth logging something about # of frees.
	s_deallocationLossAmount += allocationSize;
}

void ArgusMemorySource::CopyMemory(void* destination, void* source, SIZE_T amount)
{
	FMemory::Memcpy(destination, source, amount);
}