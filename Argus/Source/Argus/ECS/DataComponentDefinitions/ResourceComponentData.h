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
	FResourceSet m_resourceSet;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
