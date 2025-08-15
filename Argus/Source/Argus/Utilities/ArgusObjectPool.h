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
template <class PoolableType, typename Allocator>
class ArgusObjectPool
{
	using AllocatorType = Allocator;
	using SizeType = typename Allocator::SizeType;
	using ElementType = PoolableType;

	using ElementAllocatorType = std::conditional_t<
		AllocatorType::NeedsElementType,
		typename AllocatorType::template ForElementType<ElementType>,
		typename AllocatorType::ForAnyElementType>;

public:
	ArgusObjectPool() : m_capacity(m_storage.GetInitialCapacity())
	{
	}
	~ArgusObjectPool();

	PoolableType* Take();
	void Release(PoolableType*& objectPointer);
	void ClearPool();
	int32 GetNumAvailableObjects();

private:
	ElementAllocatorType m_storage;
	SizeType m_capacity = 0;
	int32 m_allocatedIndex = 0;
	ArgusDeque<PoolableType*, Allocator> m_availableObjectsPointers;
};

template <class PoolableType, typename Allocator>
ArgusObjectPool<PoolableType, Allocator>::~ArgusObjectPool()
{
	ClearPool();
}

template <class PoolableType, typename Allocator>
PoolableType* ArgusObjectPool<PoolableType, Allocator>::Take()
{
	if (m_availableObjectsPointers.Num() > 0)
	{
		PoolableType* objectPointer = m_availableObjectsPointers.First();
		m_availableObjectsPointers.PopFirst();
		return objectPointer;
	}

	if (UNLIKELY(!m_storage.HasAllocation()))
	{
		m_storage.ResizeAllocation(0, m_capacity, sizeof(PoolableType), alignof(PoolableType));
	}

	if (UNLIKELY(m_allocatedIndex >= m_capacity))
	{
		// TODO JAMES: Logwarn? Error?
		return new (ArgusMemorySource::Allocate<PoolableType>()) PoolableType();
	}

	PoolableType* output = new (&(m_storage.GetAllocation()[m_allocatedIndex])) PoolableType();
	m_allocatedIndex++;
	return output;
}

template <class PoolableType, typename Allocator>
void ArgusObjectPool<PoolableType, Allocator>::Release(PoolableType*& objectPointer)
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

template <class PoolableType, typename Allocator>
void ArgusObjectPool<PoolableType, Allocator>::ClearPool()
{
	m_availableObjectsPointers.Reset();
	m_allocatedIndex = 0;
}

template <class PoolableType, typename Allocator>
int32 ArgusObjectPool<PoolableType, Allocator>::GetNumAvailableObjects()
{
	return m_availableObjectsPointers.Num();
}
