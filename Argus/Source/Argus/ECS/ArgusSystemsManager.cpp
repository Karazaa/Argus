// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Engine/World.h"
#include "Systems/AbilitySystems.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/ConstructionSystems.h"
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

	PopulateSingletonComponents(worldPointer);
	bool didEntityPositionChangeThisFrame = false;

	TimerSystems::RunSystems(deltaTime);
	ConstructionSystems::RunSystems(deltaTime);
	NavigationSystems::RunSystems(worldPointer);
	TargetingSystems::RunSystems(deltaTime);
	AbilitySystems::RunSystems(deltaTime);
	AvoidanceSystems::RunSystems(worldPointer, deltaTime);
	didEntityPositionChangeThisFrame |= TransformSystems::RunSystems(worldPointer, deltaTime);
	didEntityPositionChangeThisFrame |= SpawningSystems::RunSystems(deltaTime);

	UpdateSingletonComponents(didEntityPositionChangeThisFrame);
}

void ArgusSystemsManager::PopulateSingletonComponents(UWorld* worldPointer)
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
		spatialPartitioningComponent->m_argusEntityKDTree.RebuildKDTreeForAllArgusEntities();

		if (worldPointer)
		{
			SpatialPartitioningSystems::CalculateAvoidanceObstacles(spatialPartitioningComponent, worldPointer);
		}
	}

	ReticleComponent* reticleComponent = singletonEntity.AddComponent<ReticleComponent>();
}

void ArgusSystemsManager::UpdateSingletonComponents(bool didEntityPositionChangeThisFrame)
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

	if (didEntityPositionChangeThisFrame)
	{
		SpatialPartitioningSystems::RunSystems(singletonEntity);
	}
}
