// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReticleActor.generated.h"

class UDecalComponent;

UCLASS()
class AReticleActor : public AActor
{
	GENERATED_BODY()

public:
	AReticleActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

	void EnableReticle(uint32 abilityRecordId);
	void DisableReticle();

	TObjectPtr<UDecalComponent> m_decalComponent = nullptr;
};