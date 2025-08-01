// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "Containers/Deque.h"

template<typename DataType, uint32 NumPreAllocatedElements>
class ArgusDeque : public TDeque<DataType, ArgusContainerAllocator<NumPreAllocatedElements> >
{
public:
	ArgusDeque& operator=(const ArgusDeque& other);
};

template<typename DataType, uint32 NumPreAllocatedElements>
ArgusDeque<DataType, NumPreAllocatedElements>& ArgusDeque<DataType, NumPreAllocatedElements>::operator=(const ArgusDeque& other)
{
	this->Empty();
	return *this;
}