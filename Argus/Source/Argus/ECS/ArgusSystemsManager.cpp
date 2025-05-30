// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Engine/World.h"
#include "Systems/AbilitySystems.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/NavigationSystems.h"
#include "Systems/ResourceSystems.h"
#include "Systems/SpatialPartitioningSystems.h"
#include "Systems/SpawningSystems.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TimerSystems.h"
#include "Systems/TransformSystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

void ArgusSystemsManager::Initialize(UWorld* worldPointer, const FResourceSet& initialTeamResourceSet)
{
	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	PopulateSingletonComponents(worldPointer);
	PopulateTeamComponents(initialTeamResourceSet);
}

void ArgusSystemsManager::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(ArgusSystemsManager::RunSystems);

	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	bool didEntityPositionChangeThisFrame = false;

	TimerSystems::RunSystems(deltaTime);
	TargetingSystems::RunSystems(deltaTime);
	AbilitySystems::RunSystems(deltaTime);
	CombatSystems::RunSystems(deltaTime);
	ResourceSystems::RunSystems(deltaTime);
	ConstructionSystems::RunSystems(deltaTime);
	NavigationSystems::RunSystems(worldPointer);
	AvoidanceSystems::RunSystems(worldPointer, deltaTime);
	didEntityPositionChangeThisFrame |= TransformSystems::RunSystems(worldPointer, deltaTime);
	didEntityPositionChangeThisFrame |= SpawningSystems::RunSystems(deltaTime);

	UpdateSingletonComponents(didEntityPositionChangeThisFrame);

#if !UE_BUILD_SHIPPING
	ArgusECSDebugger::DrawECSDebugger();
#endif //!UE_BUILD_SHIPPING
}

void ArgusSystemsManager::PopulateSingletonComponents(UWorld* worldPointer)
{
	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

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
		spatialPartitioningComponent->m_argusEntityKDTree.SeedTreeWithAverageEntityLocation();
		spatialPartitioningComponent->m_argusEntityKDTree.InsertAllArgusEntitiesIntoKDTree();

		if (worldPointer)
		{
			SpatialPartitioningSystems::CalculateAvoidanceObstacles(spatialPartitioningComponent, worldPointer);
		}
	}

	ReticleComponent* reticleComponent = singletonEntity.AddComponent<ReticleComponent>();
	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.AddComponent<InputInterfaceComponent>();
}

void ArgusSystemsManager::PopulateTeamComponents(const FResourceSet& initialTeamResourceSet)
{
	uint16 sizeOfTeamEnum = sizeof(ETeam) * 8;
	for (uint16 i = ArgusECSConstants::k_singletonEntityId - sizeOfTeamEnum; i < ArgusECSConstants::k_singletonEntityId; ++i)
	{
		if (ArgusEntity::DoesEntityExist(i))
		{
			continue;
		}

		ArgusEntity teamEntity = ArgusEntity::CreateEntity(i);
		if (!teamEntity)
		{
			continue;
		}

		ResourceComponent* teamResourceComponent = teamEntity.AddComponent<ResourceComponent>();
		if (!teamResourceComponent)
		{
			continue;
		}

		teamResourceComponent->m_currentResources.ApplyResourceChange(initialTeamResourceSet);
	}
}

void ArgusSystemsManager::UpdateSingletonComponents(bool didEntityPositionChangeThisFrame)
{
	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
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

	SpatialPartitioningSystems::RunSystems(singletonEntity, didEntityPositionChangeThisFrame);
}
