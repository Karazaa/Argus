// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/DecalSystemsArgs.h"

class UArgusActorRecord;
class UWorld;

enum class EDecalTypePolicy : uint8
{
	PopulateAttackMove,
	PopulateMoveToLocation,
	DeferredPopulation
};

class DecalSystems
{
public:
	static void RunSystems(UWorld* worldPointer, float deltaTime);

	static void DrawLineForConnectedDecals(UWorld* worldPointer, DecalSystemsArgs& components);
	static ArgusEntity InstantiateMoveToLocationDecalEntity(const UArgusActorRecord* moveToLocationDecalRecord, const FVector& targetLocation, uint16 numReferencers, uint16 connectedEntityId, EDecalTypePolicy attackMovePolicy);
	static void SetMoveToLocationDecalPerEntity(ArgusEntity owningEntity, TargetingComponent* targetingComponent, ArgusEntity decalEntity);
	static void ActivateCachedMoveToLocationDecalPerEntity(const UArgusActorRecord* moveToLocationDecalRecord, ArgusEntity entity);
	static void ClearMoveToLocationDecalPerEntity(ArgusEntity entity, bool clearQueuedWaypoints);
	static uint16 GetMostRecentSelectedWaypointDecalEntityId();

private:
	static void ActivateMoveToLocationDecalEntity(const UArgusActorRecord* moveToLocationDecalRecord, const FVector& location, uint16 connectedEntityId, uint16& decalEntityId, EDecalTypePolicy newSelectedAttackMovePolicy);
	static void ClearMoveToLocationDecalEntity(uint16& decalEntityId);
	static void ChangeToAttackMoveDecalIfNeeded(ArgusEntity selectedEntity, ArgusEntity decalEntity);
};
