// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/FlockingSystemsArgs.h"

class FlockingSystems
{
public:
	static void RunSystems(float deltaTime);
	static ArgusEntity GetFlockingRootEntity(ArgusEntity entity);
	static FlockingComponent* GetFlockingRootComponent(ArgusEntity entity);
	static FVector GetFlockingPoint(ArgusEntity flockingRootEntity);

private:
	static void ClearPackingValues();
	static void SetPackingValues();
	static void SetFlockingState(float deltaTime);

	static void StartFlockingIfNecessary(const FlockingSystemsArgs& components);
	static void EndFlockingIfNecessary(float deltaTime, const FlockingSystemsArgs& components);
	static bool PackFlockingRoot(const FlockingSystemsArgs& components);
	static float GetCurrentFlockingRootRadius(const FlockingComponent* flockingRootFlockingComponent);
	static uint16 GetCurrentFlockingRootMaxCount(const FlockingComponent* flockingRootFlockingComponent);
	static uint16 GetFlockingRootMaxCountForTier(uint8 flockingTier);

	static void IncrementStableEntitiesInRange(FlockingComponent* flockingRootFlockingComponent);
};
