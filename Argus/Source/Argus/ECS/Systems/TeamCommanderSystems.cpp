// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "Systems/TeamCommanderSystems_GatherInfo.h"
#include "Systems/TeamCommanderSystems_UpdatePriorities.h"
#include "Systems/TeamCommanderSystems_AssignEntities.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif // !UE_BUILD_SHIPPING

void TeamCommanderSystems::RunSystems(float)
{
	ARGUS_TRACE(TeamCommanderSystems::RunSystems);

	TeamCommanderSystems_GatherInfo::RunSystems();
	TeamCommanderSystems_UpdatePriorities::RunSystems();
	TeamCommanderSystems_AssignEntities::RunSystems();
}

void TeamCommanderSystems::InitializeRevealedAreas(TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float areasPerWidth = ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength);
	const int32 numAreas = FMath::FloorToInt32(FMath::Square(areasPerWidth));
	teamCommanderComponent->m_revealedAreas.SetNum(numAreas, false);
}

void TeamCommanderSystems::PerformInitialUpdate()
{
	TeamCommanderSystems_GatherInfo::RunSystems();
}

int32 TeamCommanderSystems::GetAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, -1);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return -1;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, -1);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float areasPerDimension = ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength);

	float xValue = ArgusMath::SafeDivide(components.m_transformComponent->m_location.Y + spatialPartitioningComponent->m_validSpaceExtent, teamCommanderComponent->m_revealedAreaDimensionLength);
	float yValue = ArgusMath::SafeDivide((-components.m_transformComponent->m_location.X) + spatialPartitioningComponent->m_validSpaceExtent, teamCommanderComponent->m_revealedAreaDimensionLength);

	int32 xValue32 = FMath::FloorToInt32(xValue);
	int32 yValue32 = FMath::FloorToInt32(yValue);

	return (yValue32 * FMath::FloorToInt32(areasPerDimension)) + xValue32;
}

FVector TeamCommanderSystems::GetWorldSpaceLocationFromAreaIndex(int32 areaIndex, const TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, FVector::ZeroVector);
	if (areaIndex < 0)
	{
		return FVector::ZeroVector;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, FVector::ZeroVector);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const int32 areasPerDimension = FMath::FloorToInt32(ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength));

	int32 xCoordinate, yCoordinate;
	ConvertAreaIndexToAreaCoordinates(areaIndex, areasPerDimension, xCoordinate, yCoordinate);

	const float xOffset = ((static_cast<float>(xCoordinate) + 0.5f) * teamCommanderComponent->m_revealedAreaDimensionLength);
	const float yOffset = ((static_cast<float>(yCoordinate) + 0.5f) * teamCommanderComponent->m_revealedAreaDimensionLength);

	FVector output = FVector::ZeroVector;
	output.Y = xOffset - spatialPartitioningComponent->m_validSpaceExtent;
	output.X = spatialPartitioningComponent->m_validSpaceExtent - yOffset;
	return output;
}

int32 TeamCommanderSystems::GetClosestUnrevealedAreaToEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, -1);

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, -1);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const int32 areasPerDimension = FMath::FloorToInt32(ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength));
	int32 entityXCoordinate, entityYCoordinate;
	ConvertAreaIndexToAreaCoordinates(GetAreaIndexFromWorldSpaceLocation(components, teamCommanderComponent), areasPerDimension, entityXCoordinate, entityYCoordinate);

	if (entityXCoordinate < 0 || entityYCoordinate < 0)
	{
		return -1;
	}

	for (int32 i = 1; i < areasPerDimension; ++i)
	{
		const int32 leftBoundX = FMath::Max(entityXCoordinate - i, 0);
		const int32 rightBoundX = FMath::Min(entityXCoordinate + i, (areasPerDimension - 1));
		const int32 upperBoundY = FMath::Max(entityYCoordinate - i, 0);
		const int32 lowerBoundY = FMath::Min(entityYCoordinate + i, (areasPerDimension - 1));

		TArray<int32> validAreasNearby;
		for (int32 j = -i; j <= i; ++j)
		{
			const int32 xBoundY = FMath::Min(FMath::Max(entityYCoordinate + j, 0), (areasPerDimension - 1));
			const int32 yBoundX = FMath::Min(FMath::Max(entityXCoordinate + j, 0), (areasPerDimension - 1));

			int32 indexToCheck;
			ConvertAreaCoordinatesToAreaIndex(leftBoundX, xBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(rightBoundX, xBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(yBoundX, upperBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(yBoundX, lowerBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
		}

		if (validAreasNearby.Num() > 0)
		{
			return validAreasNearby[FMath::RandRange(0, validAreasNearby.Num() - 1)];
		}
	}

	return -1;
}

void TeamCommanderSystems::ConvertAreaIndexToAreaCoordinates(int32 areaIndex, int32 areasPerDimension, int32& xCoordinate, int32& yCoordinate)
{
	if (areaIndex < 0)
	{
		xCoordinate = -1;
		yCoordinate = -1;
		return;
	}

	yCoordinate = ArgusMath::SafeDivide(areaIndex, areasPerDimension);
	xCoordinate = areaIndex % areasPerDimension;
}

void TeamCommanderSystems::ConvertAreaCoordinatesToAreaIndex(int32 xCoordinate, int32 yCoordinate, int32 areasPerDimension, int32& areaIndex)
{
	areaIndex = (yCoordinate * areasPerDimension) + xCoordinate;
}

#if !UE_BUILD_SHIPPING
void TeamCommanderSystems::DebugRevealedAreasForTeamEntityId(uint16 teamEntityId)
{
	const TeamCommanderComponent* teamCommanderComponent = ArgusEntity::RetrieveEntity(teamEntityId).GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	const WorldReferenceComponent* worldReferenceComponent = ArgusEntity::GetSingletonEntity().GetComponent<WorldReferenceComponent>();
	ARGUS_RETURN_ON_NULL(worldReferenceComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(worldReferenceComponent->m_worldPointer, ArgusECSLog);

	const float halfDimension = ArgusMath::SafeDivide(teamCommanderComponent->m_revealedAreaDimensionLength, 2.0f);
	const FVector bounds = FVector(halfDimension, halfDimension, 0.0f);
	const FVector heightAdjust = FVector(0.0f, 0.0f, 5.0f);
	teamCommanderComponent->IterateRevealedAreas(true, [worldReferenceComponent, teamCommanderComponent, &bounds, &heightAdjust](int32 areaIndex)
	{
		DrawDebugBox(worldReferenceComponent->m_worldPointer, GetWorldSpaceLocationFromAreaIndex(areaIndex, teamCommanderComponent) + heightAdjust, bounds, FColor::Green);
	});
}
#endif // !UE_BUILD_SHIPPING
