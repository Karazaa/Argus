// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "DataComponentDefinitions/ComponentData.h"

ArgusEntity UArgusEntityTemplate::MakeEntity()
{
	ArgusEntity entity = ArgusEntity::CreateEntity(static_cast<uint16>(EntityPriority));

	for (UComponentData* componentData : ComponentData)
	{
		componentData->InstantiateComponentForEntity(entity);
	}

	return entity;
}

#if WITH_EDITOR
void UArgusEntityTemplate::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	// Sudo code

	// Somehow get the object being added from FPropertyChangedEvent.
	// Iterate over existing ComponentData entries.
	// Call a UComponentData virtual method that checks if the newly added asset is the same type.
	// If any are the same type. Error out.
}
#endif