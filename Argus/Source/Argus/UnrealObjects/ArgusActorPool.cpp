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
	AArgusActor* cachedActor = m_availableObjects[classPointer].Pop();
	if (!cachedActor)
	{
		return nullptr;
	}

	cachedActor->SetHidden(false);
	return cachedActor;
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

void ArgusActorPool::Release(TObjectPtr<AArgusActor>& actorPointer)
{
	if (!actorPointer)
	{
		return;
	}

	AArgusActor* rawActorPointer = actorPointer.Get();
	if (!rawActorPointer)
	{
		return;
	}

	Release(rawActorPointer);
	actorPointer = nullptr;
}

void ArgusActorPool::ClearPool()
{
	m_availableObjects.Empty();
}
