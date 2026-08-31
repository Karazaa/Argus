// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorPool.h"
#include "ArgusActor.h"
#include "ArgusMacros.h"
#include "Engine/World.h"

UArgusActorPool::~UArgusActorPool()
{
	ClearPool();
}

void UArgusActorPool::RequestPreLoadActors(UClass* classPointer, uint16 numActors)
{
	ARGUS_MEMORY_TRACE(UArgusActorPool);

	if (!classPointer)
	{
		return;
	}

	uint16* currentRequestCount = m_preLoadRequests.Find(classPointer);
	if (currentRequestCount)
	{
		(*currentRequestCount) += numActors;
	}
	else
	{
		m_preLoadRequests.Add(classPointer, numActors);
	}
}

void UArgusActorPool::ProcessPreLoadRequests()
{
	// TODO JAMES: Move m_preLoadRequests into a queue once available objects have been subtracted.
	// Empty m_preLoadRequests.
	// Process spawn one actor from the queue
}

AArgusActor* UArgusActorPool::Take(UWorld* worldPointer, UClass* classPointer)
{
	ARGUS_MEMORY_TRACE(UArgusActorPool);

	if (!classPointer)
	{
		return nullptr;
	}

	FActorArray* actorArray = m_availableObjects.Find(classPointer);
	if (!actorArray || actorArray->m_actors.IsEmpty())
	{
		if (!worldPointer)
		{
			return nullptr;
		}

		return worldPointer->SpawnActor<AArgusActor>(classPointer);
	}

	m_numAvailableObjects--;
	AArgusActor* cachedActor = actorArray->m_actors.Pop();
	if (!cachedActor)
	{
		return nullptr;
	}

	cachedActor->Show();
	return cachedActor;
}

void UArgusActorPool::Release(AArgusActor*& actorPointer)
{
	ARGUS_MEMORY_TRACE(UArgusActorPool);

	UClass* classPointer = actorPointer->GetClass();
	if (!classPointer)
	{
		return;
	}
	
	m_numAvailableObjects++;
	actorPointer->Reset();

	FActorArray* actorArray = m_availableObjects.Find(classPointer);
	if (!actorArray)
	{
		m_availableObjects.Emplace(classPointer, TArray<TObjectPtr<AArgusActor>>()).m_actors.Add(actorPointer);
	}
	else
	{
		actorArray->m_actors.Add(actorPointer);
	}
	
	actorPointer = nullptr;
}

void UArgusActorPool::Release(TObjectPtr<AArgusActor>& actorPointer)
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

void UArgusActorPool::ClearPool()
{
	ARGUS_MEMORY_TRACE(UArgusActorPool);
	m_availableObjects.Empty();
}
