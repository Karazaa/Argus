// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "ArgusEntity.h"

void NavigationSystems::RunSystems(TWeakObjectPtr<UWorld> worldPointer, float deltaTime)
{
	if (!worldPointer.IsValid())
	{
		UE_LOG
		(
			ArgusGameLog, Error, TEXT("[%s] was invoked with an invalid %s, %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TWeakObjectPtr<UWorld>),
			ARGUS_NAMEOF(worldPointer)
		);
		return;
	}

	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		std::optional<ArgusEntity> potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity.has_value())
		{
			continue;
		}

		NavigationComponent* targetingComponent = potentialEntity->GetComponent<NavigationComponent>();
		if (!targetingComponent)
		{
			continue;
		}
	}
}