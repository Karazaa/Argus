// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "Engine/DataAsset.h"
#include "ComponentData.generated.h"

UCLASS(abstract)
class UComponentData : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual void InstantiateComponentForEntity(const ArgusEntity& entity) const {};
	virtual bool MatchesType(UComponentData* other) const { return false; }
};