// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusEntity.h"
#include "Engine/DataAsset.h"
#include "ArgusEntityTemplate.generated.h"

class UComponentData;

UCLASS()
class UArgusEntityTemplate : public UDataAsset 
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UEntityPriority EntityPriority;

	UPROPERTY(EditAnywhere)
	TArray<UComponentData*> ComponentData;

	ArgusEntity MakeEntity();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
#endif
};