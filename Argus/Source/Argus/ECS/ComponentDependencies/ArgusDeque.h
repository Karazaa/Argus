// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "Containers/Deque.h"
#include "Serialization/Archive.h"

template<typename DataType, typename Allocator>
class ArgusDeque : public TDeque<DataType, Allocator >
{
public:
	ArgusDeque& operator=(const ArgusDeque& other);
	void Serialize(FArchive& archive);
};

template<typename DataType, typename Allocator>
ArgusDeque<DataType, Allocator>& ArgusDeque<DataType, Allocator>::operator=(const ArgusDeque& other)
{
	this->Reset();
	return *this;
}

template<typename DataType, typename Allocator>
void ArgusDeque<DataType, Allocator>::Serialize(FArchive& archive)
{
	if (archive.IsSaving())
	{
		int32 numberOfElements = this->Num();
		archive << numberOfElements;
		for (DataType& element : *this)
		{
			archive << element;
		}
	}
	else if (archive.IsLoading())
	{
		this->Reset();

		int32 numberOfElements = 0;
		archive << numberOfElements;
		for (int32 i = 0; i < numberOfElements; ++i)
		{
			DataType element;
			archive << element;
			this->PushLast(element);
		}
	}
}