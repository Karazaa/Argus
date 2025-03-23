// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

enum class BaseState : uint8
{
	SpawnedWaitingForActorTake,
	Alive,
	Dead,
	DestroyedWaitingForActorRelease	
};

enum class MovementState : uint8
{
	None,
	ProcessMoveToLocationCommand,
	ProcessMoveToEntityCommand,
	MoveToLocation,
	MoveToEntity,
	AwaitingFinish,
	FailedToFindPath,
};

enum class SpawningState : uint8
{
	None,
	ProcessQueuedSpawnEntity,
	WaitingToSpawnEntity,
	SpawningEntity
};

enum class AbilityState : uint8
{
	None,
	ProcessCastAbility0Command,
	ProcessCastAbility1Command,
	ProcessCastAbility2Command,
	ProcessCastAbility3Command,
	ProcessCastReticleAbility
};

enum class ConstructionState : uint8
{
	None,
	ConstructingOther,
	BeingConstructed,
	ConstructionFinished
};

enum class CombatState : uint8
{
	None,
	ShouldAttack,
	Attacking
};