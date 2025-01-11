// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

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

	static void CalculateAvoidanceObstacles(UWorld* worldPointer);

private:
	static bool GetNavMeshWalls(const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls);
};