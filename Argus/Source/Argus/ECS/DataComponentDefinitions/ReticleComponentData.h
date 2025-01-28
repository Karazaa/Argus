// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "ReticleComponentData.generated.h"

UCLASS()
class ARGUS_API UReticleComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	FVector m_reticleLocation = FVector::ZeroVector;
	uint32 m_abilityRecordId = 0u;
	bool m_isBlocked = false;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
