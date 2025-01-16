// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ComponentDependencies/ObstaclePoint.h"

class UWorld;
class ARecastNavMesh;
struct FNavLocation;

class SpatialPartitioningSystems
{
public:
	static void RunSystems(const ArgusEntity& spatialPartitioningEntity);

	struct SpatialPartitioningSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		const TransformComponent* m_transformComponent = nullptr;
		SpatialPartitioningComponent* m_singletonSpatialParitioningComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};

	static void CalculateAvoidanceObstacles(SpatialPartitioningComponent* spatialPartitioningComponent, UWorld* worldPointer);
	static float FindAreaOfObstacleCartesian(const ObstaclePointArray& obstaclePoints);

private:
	static bool GetNavMeshWalls(const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls);
	static void ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, TArray<ObstaclePointArray>& outObstacles);
	static void CalculateDirectionAndConvexForObstacles(ObstaclePointArray& outObstacle);
	static void DrawDebugObstacles(UWorld* worldPointer, const TArray<ObstaclePointArray>& obstacles);
};