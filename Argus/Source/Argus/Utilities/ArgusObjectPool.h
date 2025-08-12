// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusLogging.h"
#include "ComponentDependencies/ArgusDeque.h"

// Example interface! We don't actually need an interface or virtual table overhead since it is used in templates, but writing it out here for convenient template API definition.
//class IObjectPoolable
//{
//	virtual void Reset() = 0;
//};

// Type IObjectPoolable is an implicit constraint for using the ArgusObjectPool. Types you want to pool should inherit from IObjectPoolable 
template <class PoolableType, uint32 NumPreAllocatedElements>
class ArgusObjectPool
{
public:
	~ArgusObjectPool();

	PoolableType* Take();
	void Release(PoolableType*& objectPointer);
	void ClearPool();
	int32 GetNumAvailableObjects();

private:
	PoolableType* m_allocatedObjects = nullptr;
	ArgusDeque<PoolableType*, NumPreAllocatedElements> m_availableObjectsPointers;
	int32 m_allocatedIndex = 0;
};

template <class PoolableType, uint32 NumPreAllocatedElements>
ArgusObjectPool<PoolableType, NumPreAllocatedElements>::~ArgusObjectPool()
{
	ClearPool();
}

template <class PoolableType, uint32 NumPreAllocatedElements>
PoolableType* ArgusObjectPool<PoolableType, NumPreAllocatedElements>::Take()
{
	if (m_availableObjectsPointers.Num() > 0)
	{
		PoolableType* objectPointer = m_availableObjectsPointers.First();
		m_availableObjectsPointers.PopFirst();
		return objectPointer;
	}

	if (UNLIKELY(!m_allocatedObjects))
	{
		m_allocatedObjects = ArgusMemorySource::AllocateRange<PoolableType>(NumPreAllocatedElements);
	}

	if (UNLIKELY(m_allocatedIndex >= NumPreAllocatedElements))
	{
		// TODO JAMES: Logwarn?
		return new (ArgusMemorySource::Allocate<PoolableType>()) PoolableType();
	}

	PoolableType* output = new (&m_allocatedObjects[m_allocatedIndex]) PoolableType();
	m_allocatedIndex++;
	return output;
}

template <class PoolableType, uint32 NumPreAllocatedElements>
void ArgusObjectPool<PoolableType, NumPreAllocatedElements>::Release(PoolableType*& objectPointer)
{
	if (!objectPointer)
	{
		ARGUS_LOG(ArgusUtilitiesLog, Error, TEXT("[%s] Attempting to release a nullptr."), ARGUS_FUNCNAME);
		return;
	}

	objectPointer->Reset();
	m_availableObjectsPointers.PushFirst(objectPointer);
	objectPointer = nullptr;
}

template <class PoolableType, uint32 NumPreAllocatedElements>
void ArgusObjectPool<PoolableType, NumPreAllocatedElements>::ClearPool()
{
	m_availableObjectsPointers.Reset();
	m_allocatedIndex = 0;
}

template <class PoolableType, uint32 NumPreAllocatedElements>
int32 ArgusObjectPool<PoolableType, NumPreAllocatedElements>::GetNumAvailableObjects()
{
	return m_availableObjectsPointers.Num();
}
