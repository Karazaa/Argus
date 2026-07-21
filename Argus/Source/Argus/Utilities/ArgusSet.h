// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Containers/Set.h"

template <typename KeyType, typename SetAllocator = FDefaultSetAllocator, typename KeyFuncs = DefaultKeyFuncs<KeyType> >
class ArgusSet : public TSet<KeyType, KeyFuncs, SetAllocator>
{
};