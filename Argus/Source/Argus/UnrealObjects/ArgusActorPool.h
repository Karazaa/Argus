// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "ComponentDependencies/ArgusDeque.h"
#include "UObject/ObjectKey.h"
#include "ArgusActorPool.generated.h"

class AArgusActor;
class UWorld;

USTRUCT()
struct FActorArray
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Transient)
	TArray<TObjectPtr<AArgusActor>> m_actors;
};

UCLASS(EditInlineNew, DefaultToInstanced)
class UArgusActorPool : public UObject
{
	GENERATED_BODY()

public:
	~UArgusActorPool();

	void RequestPreLoadActors(UClass* classPointer, uint16 numActors = 1u);
	void ProcessPreLoadRequests(UWorld* worldPointer);
	AArgusActor* Take(UWorld* worldPointer, UClass* classPointer);
	void Release(AArgusActor*& actorPointer);
	void Release(TObjectPtr<AArgusActor>& actorPointer);
	void ClearPool();
	uint32 GetNumAvailableObjects() const { return m_numAvailableObjects; }

private:
	UPROPERTY(VisibleAnywhere, Transient)
	TMap<TSubclassOf<AArgusActor>, FActorArray> m_availableObjects;

	TMap<TObjectKey<UClass>, uint16> m_preLoadRequests;
	ArgusDeque<TObjectKey<UClass>, ArgusContainerAllocator<20u>> m_committedPreLoadInstances;

	UPROPERTY(VisibleAnywhere, Transient)
	uint32 m_numAvailableObjects = 0u;
};