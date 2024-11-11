// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "CoreMinimal.h"

class UWorld;

class ArgusActorPool
{
public:
	~ArgusActorPool();

	AArgusActor* Take(UWorld* worldPointer, const TSoftClassPtr<AArgusActor>& classSoftPointer);
	void Release(AArgusActor*& actorPointer);
	void ClearPool();
	uint32 GetNumAvailableObjects() const { return m_numAvailableObjects; }

private:
	TMap<UClass*, TArray<TObjectPtr<AArgusActor>>> m_availableObjects;
	uint32 m_numAvailableObjects = 0u;
};