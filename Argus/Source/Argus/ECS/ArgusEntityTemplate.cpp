// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"

void UArgusEntityTemplate::MakeEntity()
{
	ArgusEntity entity = ArgusEntity::CreateEntity(static_cast<uint16>(EntityPriority));

	for (UComponentData* componentData : ComponentData)
	{
		componentData->InstantiateComponentForEntity(entity);
	}
}
