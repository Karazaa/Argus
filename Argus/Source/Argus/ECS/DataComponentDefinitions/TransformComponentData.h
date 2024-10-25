// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "TransformComponentData.generated.h"

UCLASS()
class ARGUS_API UTransformComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	FTransform m_transform = FTransform::Identity;
	FVector m_avoidanceVelocity = FVector::ZeroVector;
	float m_avoidanceSpeedUnitsPerSecond = 100.0f;
	UPROPERTY(EditAnywhere)
	float m_desiredSpeedUnitsPerSecond = 100.0f;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
