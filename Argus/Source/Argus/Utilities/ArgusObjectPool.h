// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include <queue>

class IObjectPoolable
{
	virtual void Reset() = 0;
};

// Type IObjectPoolable is an implicit constraint for using the ArgusObjectPool. Types you want to pool should inherit from IObjectPoolable 
template <class IObjectPoolable>
class ArgusObjectPool
{
public:
	~ArgusObjectPool();

	IObjectPoolable* Take();
	void Release(IObjectPoolable* objectPointer);
	void ClearPool();
	std::size_t GetNumAvailableObjects();

private:
	std::queue<IObjectPoolable*> m_availableObjects;

};

template <class IObjectPoolable>
ArgusObjectPool<IObjectPoolable>::~ArgusObjectPool()
{
	ClearPool();
}

template <class IObjectPoolable>
IObjectPoolable* ArgusObjectPool<IObjectPoolable>::Take()
{
	if (m_availableObjects.size() > 0)
	{
		IObjectPoolable* objectPointer = m_availableObjects.front();
		m_availableObjects.pop();
		objectPointer->Reset();
		return objectPointer;
	}

	return new IObjectPoolable();
}

template <class IObjectPoolable>
void ArgusObjectPool<IObjectPoolable>::Release(IObjectPoolable* objectPointer)
{
	if (!objectPointer)
	{
		return;
	}

	m_availableObjects.push(objectPointer);
}

template <class IObjectPoolable>
void ArgusObjectPool<IObjectPoolable>::ClearPool()
{
	while (m_availableObjects.size() > 0)
	{
		IObjectPoolable* objectPointer = m_availableObjects.front();
		m_availableObjects.pop();
		delete objectPointer;
	}
}

template <class IObjectPoolable>
std::size_t ArgusObjectPool<IObjectPoolable>::GetNumAvailableObjects()
{
	return m_availableObjects.size();
}
