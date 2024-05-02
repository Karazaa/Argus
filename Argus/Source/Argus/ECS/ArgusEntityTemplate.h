// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "Engine/DataAsset.h"
#include "UObject/SoftObjectPtr.h"
#include "ArgusEntityTemplate.generated.h"

class UComponentData;

UCLASS()
class UArgusEntityTemplate : public UDataAsset 
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UEntityPriority m_entityPriority;

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UComponentData>> m_componentData;

	ArgusEntity MakeEntity();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
#endif
};