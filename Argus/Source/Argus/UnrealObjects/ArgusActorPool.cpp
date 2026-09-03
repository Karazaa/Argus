// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorPool.h"
#include "ArgusActor.h"
#include "ArgusMacros.h"
#include "Engine/World.h"

UArgusActorPool* UArgusActorPool::s_argusActorPool = nullptr;

UArgusActorPool::UArgusActorPool()
{
	if (!s_argusActorPool)
	{
		s_argusActorPool = this;
	}
}

UArgusActorPool::~UArgusActorPool()
{
	if (s_argusActorPool == this)
	{
		s_argusActorPool = nullptr;
	}

	ClearPool();
}

void UArgusActorPool::RequestPreLoadActors(UClass* classPointer, uint16 numActors)
{
	ARGUS_RETURN_ON_NULL(classPointer, ArgusUnrealObjectsLog);
	ARGUS_MEMORY_TRACE(UArgusActorPool);

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

void UArgusActorPool::ProcessPreLoadRequests(UWorld* worldPointer)
{
	ARGUS_TRACE(UArgusActorPool::ProcessPreLoadRequests);
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusUnrealObjectsLog);
	ARGUS_MEMORY_TRACE(UArgusActorPool);

	// Commit preload requests to a queue once available objects have been subtracted.
	for (const TPair<TObjectKey<UClass>, uint16>& preLoadRequest : m_preLoadRequests)
	{
		UClass* classPointer = preLoadRequest.Key.ResolveObjectPtr();
		if (!classPointer)
		{
			continue;
		}

		uint16 numActorsToPreload = preLoadRequest.Value;
		if (const FActorArray* availableActors = m_availableObjects.Find(classPointer))
		{
			numActorsToPreload -= static_cast<uint16>(availableActors->m_actors.Num());
		}

		for (uint16 i = 0; i < numActorsToPreload; ++i)
		{
			m_committedPreLoadInstances.PushLast(preLoadRequest.Key);
		}
	}

	// Reset preload requests for next frame
	m_preLoadRequests.Reset();

	if (m_committedPreLoadInstances.IsEmpty())
	{
		return;
	}

	// Spawn one actor from the queue
	if (UClass* classToSpawn = m_committedPreLoadInstances.First().ResolveObjectPtr())
	{
		if (AArgusActor* spawnedActor = worldPointer->SpawnActor<AArgusActor>(classToSpawn))
		{
			m_numAvailableObjects++;
			spawnedActor->Reset();

			FActorArray* actorArray = m_availableObjects.Find(classToSpawn);
			if (!actorArray)
			{
				m_availableObjects.Emplace(classToSpawn, TArray<TObjectPtr<AArgusActor>>()).m_actors.Add(spawnedActor);
			}
			else
			{
				actorArray->m_actors.Add(spawnedActor);
			}
		}
	}

	m_committedPreLoadInstances.PopFirst();
}

AArgusActor* UArgusActorPool::Take(UWorld* worldPointer, UClass* classPointer)
{
	ARGUS_RETURN_ON_NULL_POINTER(classPointer, ArgusUnrealObjectsLog);
	ARGUS_RETURN_ON_NULL_POINTER(worldPointer, ArgusUnrealObjectsLog);
	ARGUS_MEMORY_TRACE(UArgusActorPool);

	FActorArray* actorArray = m_availableObjects.Find(classPointer);
	if (!actorArray || actorArray->m_actors.IsEmpty())
	{
		return worldPointer->SpawnActor<AArgusActor>(classPointer);
	}

	m_numAvailableObjects--;
	AArgusActor* cachedActor = actorArray->m_actors.Pop();
	ARGUS_RETURN_ON_NULL_POINTER(cachedActor, ArgusUnrealObjectsLog);

	cachedActor->Show();
	return cachedActor;
}

void UArgusActorPool::Release(AArgusActor*& actorPointer)
{
	ARGUS_RETURN_ON_NULL(actorPointer, ArgusUnrealObjectsLog);
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
