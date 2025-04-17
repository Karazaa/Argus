// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "ResourceExtractionComponentData.generated.h"

UCLASS()
class ARGUS_API UResourceExtractionComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FResourceSet m_resourcesToExtract;
	UPROPERTY(EditAnywhere)
	float m_extractionLengthSeconds = 1.0f;
	TimerHandle m_resourceExtractionTimer;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
