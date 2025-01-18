//Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Containers/Queue.h"

template<typename DataType>
class ArgusQueue : public TQueue<DataType>
{
public:
	ArgusQueue& operator=(const ArgusQueue& other);
};

template<typename DataType>
ArgusQueue<DataType>& ArgusQueue<DataType>::operator=(const ArgusQueue& other)
{
	this->Empty();
	return *this;
}