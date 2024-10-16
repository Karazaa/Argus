// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include <queue>

template <class T>
class ArgusObjectPool
{
public:
	~ArgusObjectPool();

	T* Take();
	void Release(T* objectPointer);
	void ClearPool();

private:
	std::queue<T*> m_availableObjects;

};

template <class T>
ArgusObjectPool<T>::~ArgusObjectPool()
{
	ClearPool();
}

template <class T>
void ArgusObjectPool<T>::ClearPool()
{
	while (m_availableObjects.size() > 0)
	{
		T* objectPointer = m_availableObjects.front();
		m_availableObjects.pop();
		delete objectPointer;
	}
}

template <class T>
T* ArgusObjectPool<T>::Take()
{
	if (m_availableObjects.size() > 0)
	{
		T* objectPointer = m_availableObjects.front();
		m_availableObjects.pop();
		return objectPointer;
	}

	return new T();
}

template <class T>
void ArgusObjectPool<T>::Release(T* objectPointer)
{
	if (!objectPointer)
	{
		return;
	}

	m_availableObjects.push(objectPointer);
}
