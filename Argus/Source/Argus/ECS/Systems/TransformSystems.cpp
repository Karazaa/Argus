// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"

void TransformSystems::RunSystems(float deltaTime)
{
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		std::optional<ArgusEntity> potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity.has_value())
		{
			continue;
		}

		TransformComponent* transformComponent = potentialEntity->GetComponent<TransformComponent>();
		if (!transformComponent)
		{
			continue;
		}

		// TODO JAMES: Run individual systems per entity below.
	}
}