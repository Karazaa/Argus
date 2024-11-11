// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorPool.h"
#include "Engine/World.h"

ArgusActorPool::~ArgusActorPool()
{
	ClearPool();
}

AArgusActor* ArgusActorPool::Take(UWorld* worldPointer, const TSoftClassPtr<AArgusActor>& classSoftPointer)
{
	if (!classSoftPointer)
	{
		return nullptr;
	}

	UClass* classPointer = classSoftPointer.LoadSynchronous();
	if (!classPointer)
	{
		return nullptr;
	}

	if (!m_availableObjects.Contains(classPointer) || m_availableObjects[classPointer].IsEmpty())
	{
		if (!worldPointer)
		{
			return nullptr;
		}

		return worldPointer->SpawnActor<AArgusActor>(classPointer);
	}

	m_numAvailableObjects--;
	return m_availableObjects[classPointer].Pop();
}

void ArgusActorPool::Release(AArgusActor*& actorPointer)
{
	UClass* classPointer = actorPointer->GetClass();
	if (!classPointer)
	{
		return;
	}
	
	m_numAvailableObjects++;
	actorPointer->Reset();
	if (!m_availableObjects.Contains(classPointer))
	{
		m_availableObjects.Emplace(classPointer, TArray<TObjectPtr<AArgusActor>>());
	}
	m_availableObjects[classPointer].Add(actorPointer);	
	actorPointer = nullptr;
}

void ArgusActorPool::ClearPool()
{
	m_availableObjects.Empty();
}
