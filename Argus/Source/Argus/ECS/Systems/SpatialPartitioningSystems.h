// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class ARecastNavMesh;
class UWorld;
struct FNavLocation;
struct FObstaclePointArray;
struct FObstaclesContainer;

class SpatialPartitioningSystems
{
public:
	static void RunSystems();

	ARGUS_API static void GatherAvoidanceObstacles(UWorld* worldPointer, const FVector& queryOrigin, float queryExtent, FObstaclesContainer& outObstacles);
	static void InitializeAvoidanceObstacles(SpatialPartitioningComponent* spatialPartitioningComponent, UWorld* worldPointer);
	static float FindAreaOfObstacleCartesian(const FObstaclePointArray& obstaclePoints);
	static bool IsEntityInLineOfSightOfOther(ArgusEntity sourceEntity, ArgusEntity targetEntity);
	static bool IsPointInLineOfSightOfEntity(ArgusEntity sourceEntity, const FVector& targetLocation);
	static bool AnyObstaclesOrStaticEntitiesInCircle(const FVector& center, float radius, float resourceSourceBufferRadius);
	static void CalculateAdjacentEntityGroupsForEntity(ArgusEntity entity, bool allowNavigationRecalculation);

private:
	static void ClearSeenByStatus();
	static void CacheAdjacentEntityIds(const SpatialPartitioningComponent* spatialPartitioningComponent);

	static void CalculateAdjacentEntityGroups();
	static bool FloodFillGroupRecursive(uint16 groupId, AvoidanceGroupingComponent* groupLeaderComponent, uint16 argusEntityId, uint16 lastArgusEntityId, FVector& currentPositionSum, float& numberOfEntitiesInGroup, uint16& numberOfStoppedEntities);
	static void OnBecomeAvoidanceGroupLeader(ArgusEntity entity);
	static void OnChangeAvoidanceGroups(ArgusEntity entity, AvoidanceGroupingComponent* groupingComponent);

	static bool GetNavMeshWalls(float queryExtent, const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls);
	static void ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, FObstaclesContainer& outObstacles);
	static void CalculateFixupDirectionForObstacles(FObstaclePointArray& outObstacle);
	static void ApplyFixupDirectionForObstacles(FObstaclePointArray& outObstacle);
	static void CalculateDirectionAndConvexForObstacles(FObstaclePointArray& outObstacle);


#if !UE_BUILD_SHIPPING
	static void DrawDebugObstacles(UWorld* worldPointer, const FObstaclesContainer& obstacles);
#endif //!UE_BUILD_SHIPPING
};