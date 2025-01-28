// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReticleActor.generated.h"

class UAbilityRecord;
class UDecalComponent;

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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

	void EnableReticleDecalComponent(uint32 abilityRecordId);
	void EnableReticleDecalComponentForConstruction(const UAbilityRecord* abilityRecord);
	void EnableReticleDecalComponentForAbility(const UAbilityRecord* abilityRecord);
	void DisableReticleDecalComponent();

	TObjectPtr<UDecalComponent> m_decalComponent = nullptr;
};