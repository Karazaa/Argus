// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "ComponentDependencies/ObstaclePoint.h"
#include "ObstaclesRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UObstaclesRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FObstaclesContainer m_obstaclesContainer;
};