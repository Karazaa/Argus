// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/DecalSystemsArgs.h"

class UArgusActorRecord;

class DecalSystems
{
public:
	static void RunSystems(float deltaTime);

	static ArgusEntity InstantiateMoveToLocationDecalEntity(const UArgusActorRecord* moveToLocationDecalRecord, const FVector& targetLocation, uint16 numReferencers);
	static void SetMoveToLocationDecalPerEntity(TargetingComponent* targetingComponent, ArgusEntity decalEntity);
	static void ActivateCachedMoveToLocationDecalPerEntity(const UArgusActorRecord* moveToLocationDecalRecord, ArgusEntity entity);
	static void ClearMoveToLocationDecalPerEntity(ArgusEntity entity);
};
