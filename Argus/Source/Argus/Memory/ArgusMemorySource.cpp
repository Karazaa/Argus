// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusMemorySource.h"

void* ArgusMemorySource::s_RawDataRoot = nullptr;

void ArgusMemorySource::Initialize(SIZE_T memorySourceSize, uint32 alignment)
{
	if (s_RawDataRoot)
	{
		return;
	}

	s_RawDataRoot = FMemory::Malloc(memorySourceSize, alignment);
}

void ArgusMemorySource::TearDown()
{
	if (!s_RawDataRoot)
	{
		return;
	}

	FMemory::Free(s_RawDataRoot);
	s_RawDataRoot = nullptr;
}