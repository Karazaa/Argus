// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "..\ComponentDefinitions\IdentityComponent.h"
#include "IdentityComponentData.generated.h"

UCLASS()
class ARGUS_API UIdentityComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EFaction m_faction;
	UPROPERTY(EditAnywhere)
	uint8 m_allies;
	UPROPERTY(EditAnywhere)
	uint8 m_enemies;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
