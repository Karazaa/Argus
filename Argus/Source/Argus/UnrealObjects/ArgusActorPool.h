// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "CoreMinimal.h"
#include "ArgusActorPool.generated.h"

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

	AArgusActor* Take(UWorld* worldPointer, UClass* classSoftPointer);
	void Release(AArgusActor*& actorPointer);
	void Release(TObjectPtr<AArgusActor>& actorPointer);
	void ClearPool();
	uint32 GetNumAvailableObjects() const { return m_numAvailableObjects; }

private:
	UPROPERTY(VisibleAnywhere, Transient)
	TMap<UClass*, FActorArray> m_availableObjects;

	UPROPERTY(VisibleAnywhere, Transient)
	uint32 m_numAvailableObjects = 0u;
};