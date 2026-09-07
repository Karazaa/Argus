// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "RecordReferences/ObstaclesRecordReference.h"
#include "WorldCellRecord.generated.h"

class UWorld;

UCLASS(BlueprintType)
class ARGUS_API UWorldCellRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> m_worldReference = nullptr;

	UPROPERTY(EditAnywhere)
	FUObstaclesRecordReference m_obstaclesRecord;

	UPROPERTY(EditAnywhere)
	int32 m_cellXCoordinate = 0;

	UPROPERTY(EditAnywhere)
	int32 m_cellYCoordinate = 0;

	virtual void OnAsyncLoaded() const override;
};