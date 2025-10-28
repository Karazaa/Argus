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
	static void RunSystems();
	static void CalculateAvoidanceObstacles(SpatialPartitioningComponent* spatialPartitioningComponent, UWorld* worldPointer);
	static float FindAreaOfObstacleCartesian(const ObstaclePointArray& obstaclePoints);

private:
	static void ClearSeenByStatus();
	static void CacheAdjacentEntityIds(const SpatialPartitioningComponent* spatialPartitioningComponent);
	static void CalculateAdjacentEntityGroups();
	static bool FloodFillGroupRecursive(uint16 groupId, uint16 argusEntityId, FVector& currentPositionSum, float& numberOfEntitiesInGroup, uint16& numberOfStoppedEntities, TArray<ArgusEntity>& entitiesInGroup);
	static bool GetNavMeshWalls(const SpatialPartitioningComponent* spatialPartitioningComponent, const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls);
	static void ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, TArray<ObstaclePointArray>& outObstacles);
	static void CalculateDirectionAndConvexForObstacles(ObstaclePointArray& outObstacle);
	static void DrawDebugObstacles(UWorld* worldPointer, const TArray<ObstaclePointArray>& obstacles);
};