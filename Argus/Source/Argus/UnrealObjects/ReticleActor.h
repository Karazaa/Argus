// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReticleActor.generated.h"

class UAbilityRecord;
class UDecalComponent;
struct ReticleComponent;

UCLASS()
class AReticleActor : public AActor
{
	GENERATED_BODY()

public:
	AReticleActor();

	UPROPERTY(EditAnywhere);
	FLinearColor m_validReticleColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere);
	FLinearColor m_invalidReticleColor = FLinearColor::Red;

	void UpdateReticle();

protected:
	void EnableReticleDecalComponent(const ReticleComponent* reticleComponent);
	void DisableReticleDecalComponent();

	TObjectPtr<UDecalComponent> m_decalComponent = nullptr;
};