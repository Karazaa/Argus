// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Engine/World.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/NavigationSystems.h"
#include "Systems/SpawningSystems.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TransformSystems.h"

const uint16 ArgusSystemsManager::s_singletonEntityId = ArgusECSConstants::k_maxEntities - 1u;

void ArgusSystemsManager::RunSystems(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime)
{
	ARGUS_TRACE(ArgusSystemsManager::RunSystems)

	if (!worldPointer.IsValid())
	{
		UE_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s, %s."),
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(TWeakObjectPtr<UWorld>), 
			ARGUS_NAMEOF(worldPointer)
		);
		return;
	}

	SpawningSystems::RunSystems(deltaTime);
	NavigationSystems::RunSystems(worldPointer);
	TargetingSystems::RunSystems(deltaTime);
	AvoidanceSystems::RunSystems(worldPointer, deltaTime);
	const bool didMovementUpdateThisFrame = TransformSystems::RunSystems(worldPointer, deltaTime);

	UpdateSingletonComponents(didMovementUpdateThisFrame);
}

void ArgusSystemsManager::UpdateSingletonComponents(bool didMovementUpdateThisFrame)
{
	bool createdThisFrame = false;
	if (!ArgusEntity::DoesEntityExist(s_singletonEntityId))
	{
		ArgusEntity::CreateEntity(s_singletonEntityId);
		createdThisFrame = true;
	}

	ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(s_singletonEntityId);

	if (SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetOrAddComponent<SpatialPartitioningComponent>())
	{
		if (didMovementUpdateThisFrame || createdThisFrame)
		{
			spatialPartitioningComponent->m_argusKDTree.RebuildKDTreeForAllArgusEntities();
		}
	}
}
