// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "..\ComponentDefinitions\HealthComponent.h"
#include "HealthComponentData.generated.h"

UCLASS()
class ARGUS_API UHealthComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	uint32 m_health = 1000u;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
