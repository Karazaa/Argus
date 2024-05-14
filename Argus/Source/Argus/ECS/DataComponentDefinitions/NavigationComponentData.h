// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "NavigationComponentData.generated.h"

UCLASS()
class ARGUS_API UNavigationComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	std::vector<FVector> m_navigationPoints = std::vector<FVector>();
	uint32 m_lastPointIndex = 0u;
	UPROPERTY(EditAnywhere)
	float m_navigationSpeedUnitsPerSecond = 100.0f;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
