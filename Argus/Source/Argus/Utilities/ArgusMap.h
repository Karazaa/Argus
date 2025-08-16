// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Containers/Map.h"

template <typename KeyType, typename ValueType, typename SetAllocator = FDefaultSetAllocator, typename KeyFuncs = TDefaultMapHashableKeyFuncs<KeyType,ValueType,false> >
class ArgusMap : public TMap<KeyType, ValueType, SetAllocator, KeyFuncs>
{
public:
	template <class PredicateClass>
	int32 RemoveAll(PredicateClass&& predicate)
	{
		int32 numRemovedPairs = 0;
		for (typename TMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>::ElementSetType::TIterator it(TMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>::Pairs); it; ++it)
		{
			if (Forward<PredicateClass>(predicate)(it->Key, it->Value))
			{
				it.RemoveCurrent();
				++numRemovedPairs;
			}
		}
		return numRemovedPairs;
	}
};