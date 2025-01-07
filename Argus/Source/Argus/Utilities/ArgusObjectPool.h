// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include <queue>

class IObjectPoolable
{
	virtual void Reset() = 0;
};

// Type IObjectPoolable is an implicit constraint for using the ArgusObjectPool. Types you want to pool should inherit from IObjectPoolable 
template <class PoolableType>
class ArgusObjectPool
{
public:
	~ArgusObjectPool();

	PoolableType* Take();
	void Release(PoolableType*& objectPointer);
	void ClearPool();
	std::size_t GetNumAvailableObjects();

private:
	std::queue<PoolableType*> m_availableObjects;

};

template <class PoolableType>
ArgusObjectPool<PoolableType>::~ArgusObjectPool()
{
	ClearPool();
}

template <class PoolableType>
PoolableType* ArgusObjectPool<PoolableType>::Take()
{
	if (m_availableObjects.size() > 0)
	{
		PoolableType* objectPointer = m_availableObjects.front();
		m_availableObjects.pop();
		return objectPointer;
	}

	return new PoolableType();
}

template <class PoolableType>
void ArgusObjectPool<PoolableType>::Release(PoolableType*& objectPointer)
{
	if (!objectPointer)
	{
		ARGUS_LOG(ArgusUtilitiesLog, Error, TEXT("[%s] Attempting to release a nullptr."), ARGUS_FUNCNAME);
		return;
	}

	objectPointer->Reset();
	m_availableObjects.push(objectPointer);
	objectPointer = nullptr;
}

template <class PoolableType>
void ArgusObjectPool<PoolableType>::ClearPool()
{
	while (m_availableObjects.size() > 0)
	{
		PoolableType* objectPointer = m_availableObjects.front();
		m_availableObjects.pop();
		delete objectPointer;
	}
}

template <class PoolableType>
std::size_t ArgusObjectPool<PoolableType>::GetNumAvailableObjects()
{
	return m_availableObjects.size();
}
