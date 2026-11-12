// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "DataComponentDefinitions/SpatialPartitioningComponentData.h"
#include "Engine/DataAsset.h"

#include "ArgusEditorDataLookups.generated.h"

UCLASS()
class UArgusEditorDataLookups : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USpatialPartitioningComponentData> m_spatialPartitioningComponentData = nullptr;
};