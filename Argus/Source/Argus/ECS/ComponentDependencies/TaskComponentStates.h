// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EBaseState : uint8
{
	SpawnedWaitingForActorTake,
	Alive,
	Dead,
	DestroyedWaitingForActorRelease	
};

UENUM()
enum class EMovementState : uint8
{
	None,
	ProcessMoveToLocationCommand,
	ProcessMoveToEntityCommand,
	MoveToLocation,
	MoveToEntity,
	AwaitingFinish,
	FailedToFindPath,
};

UENUM()
enum class ESpawningState : uint8
{
	None,
	ProcessQueuedSpawnEntity,
	WaitingToSpawnEntity,
	SpawningEntity
};

UENUM()
enum class EAbilityState : uint8
{
	None,
	ProcessCastAbility0Command,
	ProcessCastAbility1Command,
	ProcessCastAbility2Command,
	ProcessCastAbility3Command,
	ProcessCastReticleAbility
};

UENUM()
enum class EConstructionState : uint8
{
	None,
	ConstructingOther,
	BeingConstructed,
	ConstructionFinished
};

UENUM()
enum class ECombatState : uint8
{
	None,
	ShouldAttack,
	Attacking
};