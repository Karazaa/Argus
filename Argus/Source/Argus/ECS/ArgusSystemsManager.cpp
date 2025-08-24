// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusEntityTemplate.h"
#include "ArgusLogging.h"
#include "Engine/World.h"
#include "Systems/AbilitySystems.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/FogOfWarSystems.h"
#include "Systems/NavigationSystems.h"
#include "Systems/ResourceSystems.h"
#include "Systems/SpatialPartitioningSystems.h"
#include "Systems/SpawningSystems.h"
#include "Systems/TaskSystems.h"
#include "Systems/TimerSystems.h"
#include "Systems/TransformSystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#include "ArgusMemoryDebugger.h"
#endif //!UE_BUILD_SHIPPING

void ArgusSystemsManager::Initialize(UWorld* worldPointer, const FResourceSet& initialTeamResourceSet, const UArgusEntityTemplate* singletonTemplate)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	PopulateSingletonComponents(worldPointer, singletonTemplate);
	PopulateTeamComponents(initialTeamResourceSet);
}

void ArgusSystemsManager::OnStartPlay(UWorld* worldPointer)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	SetInitialSingletonState(worldPointer);
}

void ArgusSystemsManager::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(ArgusSystemsManager::RunSystems);
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	bool didEntityPositionChangeThisFrame = false;

	TimerSystems::RunSystems(deltaTime);
	TaskSystems::RunSystems(deltaTime);
	AbilitySystems::RunSystems(deltaTime);
	CombatSystems::RunSystems(deltaTime);
	ResourceSystems::RunSystems(deltaTime);
	ConstructionSystems::RunSystems(deltaTime);
	NavigationSystems::RunSystems(worldPointer);
	AvoidanceSystems::RunSystems(worldPointer, deltaTime);
	didEntityPositionChangeThisFrame |= TransformSystems::RunSystems(worldPointer, deltaTime);
	didEntityPositionChangeThisFrame |= SpawningSystems::RunSystems(deltaTime);
	FogOfWarSystems::RunSystems(deltaTime);

	UpdateSingletonComponents(didEntityPositionChangeThisFrame);

#if !UE_BUILD_SHIPPING
	ArgusECSDebugger::DrawECSDebugger();
	ArgusMemoryDebugger::DrawMemoryDebugger();
#endif //!UE_BUILD_SHIPPING
}

void ArgusSystemsManager::PopulateSingletonComponents(UWorld* worldPointer, const UArgusEntityTemplate* singletonTemplate)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	ArgusEntity singletonEntity = ArgusEntity::k_emptyEntity;
	if (singletonTemplate)
	{
		singletonEntity = singletonTemplate->MakeEntity(ArgusECSConstants::k_singletonEntityId);
	}
	else
	{
		singletonEntity = ArgusEntity::CreateEntity(ArgusECSConstants::k_singletonEntityId);
	}

	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] There is no singleton %s when it should have already been made."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetOrAddComponent<SpatialPartitioningComponent>();
	ReticleComponent* reticleComponent = singletonEntity.GetOrAddComponent<ReticleComponent>();
	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetOrAddComponent<InputInterfaceComponent>();
	AssetLoadingComponent* assetLoadingComponent = singletonEntity.GetOrAddComponent<AssetLoadingComponent>();

	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(reticleComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(assetLoadingComponent, ArgusECSLog);
}

void ArgusSystemsManager::SetInitialSingletonState(UWorld* worldPointer)
{
	ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId);
	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] There is no singleton %s when it should have already been made."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	spatialPartitioningComponent->m_argusEntityKDTree.SeedTreeWithAverageEntityLocation();
	spatialPartitioningComponent->m_argusEntityKDTree.InsertAllArgusEntitiesIntoKDTree();
	SpatialPartitioningSystems::CalculateAvoidanceObstacles(spatialPartitioningComponent, worldPointer);

	FogOfWarSystems::InitializeSystems();
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

		ResourceComponent* teamResourceComponent = ArgusEntity::CreateEntity(i).GetOrAddComponent<ResourceComponent>();
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
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	SpatialPartitioningSystems::RunSystems(singletonEntity, didEntityPositionChangeThisFrame);
}
