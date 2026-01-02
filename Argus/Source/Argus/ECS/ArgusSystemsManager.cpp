// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusEntityTemplate.h"
#include "ArgusLogging.h"
#include "Engine/World.h"
#include "Systems/AbilitySystems.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/DecalSystems.h"
#include "Systems/FlockingSystems.h"
#include "Systems/FogOfWarSystems.h"
#include "Systems/NavigationSystems.h"
#include "Systems/ResourceSystems.h"
#include "Systems/SpatialPartitioningSystems.h"
#include "Systems/SpawningSystems.h"
#include "Systems/TaskSystems.h"
#include "Systems/TeamCommanderSystems.h"
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

void ArgusSystemsManager::OnStartPlay(UWorld* worldPointer, ETeam activePlayerTeam)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	SetInitialSingletonState(worldPointer, activePlayerTeam);
}

void ArgusSystemsManager::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(ArgusSystemsManager::RunSystems);
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	bool didEntityPositionChangeThisFrame = false;

	TimerSystems::RunSystems(deltaTime);
	TaskSystems::RunSystems(deltaTime);
	TeamCommanderSystems::RunSystems(deltaTime);
	AbilitySystems::RunSystems(deltaTime);
	CombatSystems::RunSystems(deltaTime);
	ResourceSystems::RunSystems(deltaTime);
	ConstructionSystems::RunSystems(deltaTime);
	NavigationSystems::RunSystems(worldPointer);
	AvoidanceSystems::RunSystems(worldPointer, deltaTime);
	didEntityPositionChangeThisFrame |= TransformSystems::RunSystems(worldPointer, deltaTime);
	FlockingSystems::RunSystems(deltaTime);
	didEntityPositionChangeThisFrame |= SpawningSystems::RunSystems(deltaTime);
	DecalSystems::RunSystems(worldPointer, deltaTime);

#if !UE_BUILD_SHIPPING
	ArgusECSDebugger::DrawECSDebugger();
	ArgusMemoryDebugger::DrawMemoryDebugger();
#endif //!UE_BUILD_SHIPPING
}

void ArgusSystemsManager::RunPostThreadSystems()
{
	ARGUS_TRACE(ArgusSystemsManager::RunPostThreadSystems);
	FogOfWarSystems::RunSystems();
	SpatialPartitioningSystems::RunSystems();
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

	AssetLoadingComponent* assetLoadingComponent = singletonEntity.GetOrAddComponent<AssetLoadingComponent>();
	FogOfWarComponent* fogOfWarComponent = singletonEntity.GetOrAddComponent<FogOfWarComponent>();
	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetOrAddComponent<InputInterfaceComponent>();
	ReticleComponent* reticleComponent = singletonEntity.GetOrAddComponent<ReticleComponent>();
	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetOrAddComponent<SpatialPartitioningComponent>();

	ARGUS_RETURN_ON_NULL(assetLoadingComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(reticleComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);
}

void ArgusSystemsManager::SetInitialSingletonState(UWorld* worldPointer, ETeam activePlayerTeam)
{
	ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId);
	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] There is no singleton %s when it should have already been made."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	spatialPartitioningComponent->m_argusEntityKDTree.SeedTreeWithAverageEntityLocation(false);
	spatialPartitioningComponent->m_argusEntityKDTree.InsertAllArgusEntitiesIntoKDTree(false);
	spatialPartitioningComponent->m_flyingArgusEntityKDTree.SeedTreeWithAverageEntityLocation(true);
	spatialPartitioningComponent->m_flyingArgusEntityKDTree.InsertAllArgusEntitiesIntoKDTree(true);
	SpatialPartitioningSystems::CalculateAvoidanceObstacles(spatialPartitioningComponent, worldPointer);

	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	inputInterfaceComponent->m_activePlayerTeam = activePlayerTeam;
	inputInterfaceComponent->m_controlGroups.SetNumZeroed(inputInterfaceComponent->m_numControlGroups);
	FogOfWarSystems::InitializeSystems();
}

void ArgusSystemsManager::PopulateTeamComponents(const FResourceSet& initialTeamResourceSet)
{
	for (uint8 i = 1u; i <= (sizeof(ETeam) * 8u); ++i)
	{
		if (ArgusEntity::DoesEntityExist(ArgusECSConstants::k_singletonEntityId - i))
		{
			continue;
		}

		ArgusEntity teamEntity = ArgusEntity::CreateEntity(ArgusECSConstants::k_singletonEntityId - i);
		ResourceComponent* teamResourceComponent = teamEntity.GetOrAddComponent<ResourceComponent>();
		TeamCommanderComponent* teamCommanderComponent = teamEntity.GetOrAddComponent<TeamCommanderComponent>();
		if (!teamResourceComponent || !teamCommanderComponent)
		{
			continue;
		}

		teamResourceComponent->m_currentResources.ApplyResourceChange(initialTeamResourceSet);
		teamCommanderComponent->m_teamToCommand = static_cast<ETeam>(1u << (i - 1u));
	}
}
