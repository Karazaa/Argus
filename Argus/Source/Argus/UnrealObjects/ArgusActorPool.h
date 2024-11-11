// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "CoreMinimal.h"

class ArgusActorPool
{
public:
	~ArgusActorPool();

	AArgusActor* Take(TSoftClassPtr<AArgusActor>& classSoftPointer);
	void Release(AArgusActor*& actorPointer);
	void ClearPool();

private:
	TMap<UClass*, TArray<TObjectPtr<AArgusActor>>> m_availableObjects;
};