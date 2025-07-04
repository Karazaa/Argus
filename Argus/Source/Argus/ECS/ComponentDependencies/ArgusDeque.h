// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Containers/Deque.h"

template<typename DataType>
class ArgusDeque : public TDeque<DataType>
{
public:
	ArgusDeque& operator=(const ArgusDeque& other);
};

template<typename DataType>
ArgusDeque<DataType>& ArgusDeque<DataType>::operator=(const ArgusDeque& other)
{
	this->Empty();
	return *this;
}