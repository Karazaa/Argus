// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "Containers/Deque.h"

template<typename DataType, typename Allocator>
class ArgusDeque : public TDeque<DataType, Allocator >
{
public:
	ArgusDeque& operator=(const ArgusDeque& other);
};

template<typename DataType, typename Allocator>
ArgusDeque<DataType, Allocator>& ArgusDeque<DataType, Allocator>::operator=(const ArgusDeque& other)
{
	this->Reset();
	return *this;
}