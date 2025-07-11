// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "ResourceComponentData.generated.h"

UCLASS()
class ARGUS_API UResourceComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FResourceSet m_currentResources;
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UResourceSetRecord> m_resourceCapacityRecordId;
	UPROPERTY(EditAnywhere)
	EResourceComponentOwnerType m_resourceComponentOwnerType = EResourceComponentOwnerType::TeamPool;

	void InstantiateComponentForEntity(const ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
