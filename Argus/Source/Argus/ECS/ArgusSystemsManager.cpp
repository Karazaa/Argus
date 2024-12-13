// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Engine/World.h"
#include "Systems/AbilitySystems.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/NavigationSystems.h"
#include "Systems/SpatialPartitioningSystems.h"
#include "Systems/SpawningSystems.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TimerSystems.h"
#include "Systems/TransformSystems.h"

void ArgusSystemsManager::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(ArgusSystemsManager::RunSystems);

	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	PopulateSingletonComponents();

	TimerSystems::RunSystems(deltaTime);
	NavigationSystems::RunSystems(worldPointer);
	TargetingSystems::RunSystems(deltaTime);
	AbilitySystems::RunSystems(deltaTime);
	SpawningSystems::RunSystems(deltaTime);
	AvoidanceSystems::RunSystems(worldPointer, deltaTime);
	const bool didMovementUpdateThisFrame = TransformSystems::RunSystems(worldPointer, deltaTime);

	UpdateSingletonComponents(didMovementUpdateThisFrame);
}

void ArgusSystemsManager::PopulateSingletonComponents()
{
	if (ArgusEntity::DoesEntityExist(ArgusECSConstants::k_singletonEntityId))
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusECSConstants::k_singletonEntityId);
	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] There is no singleton %s when it should have already been made."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return;
	}

	if (SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.AddComponent<SpatialPartitioningComponent>())
	{
		spatialPartitioningComponent->m_argusKDTree.RebuildKDTreeForAllArgusEntities();
	}
}

void ArgusSystemsManager::UpdateSingletonComponents(bool didMovementUpdateThisFrame)
{
	ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId);
	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] There is no singleton %s when it should have already been made."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] There is not %s when it should have already been made."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpatialPartitioningComponent));
		return;
	}

	if (didMovementUpdateThisFrame)
	{
		SpatialPartitioningSystems::RunSystems(singletonEntity);
	}
}
