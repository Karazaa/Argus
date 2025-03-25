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
	static void CalculateAvoidanceObstacles(SpatialPartitioningComponent* spatialPartitioningComponent, UWorld* worldPointer);
	static float FindAreaOfObstacleCartesian(const ObstaclePointArray& obstaclePoints);

private:
	static void CacheAdjacentEntityIds(const SpatialPartitioningComponent* spatialPartitioningComponent);
	static void CalculateAdjacentEntityGroups();
	static bool GetNavMeshWalls(const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls);
	static void ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, TArray<ObstaclePointArray>& outObstacles);
	static void CalculateDirectionAndConvexForObstacles(ObstaclePointArray& outObstacle);
	static void DrawDebugObstacles(UWorld* worldPointer, const TArray<ObstaclePointArray>& obstacles);
};