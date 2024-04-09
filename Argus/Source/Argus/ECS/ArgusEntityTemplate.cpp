// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "ArgusUtil.h"
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
	if (propertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	const FString propertyName = propertyChangedEvent.GetPropertyName().ToString();
	const FString componentDataPropertyName = ARGUS_NAMEOF(ComponentData);
	if (!propertyName.Equals(componentDataPropertyName))
	{
		return;
	}

	const int32 arrayIndex = propertyChangedEvent.GetArrayIndex(propertyName);
	const UComponentData* modifiedComponent = ComponentData[arrayIndex];
	if (modifiedComponent)
	{
		for (int i = 0; i < ComponentData.Num(); ++i)
		{
			if (i == arrayIndex)
			{
				continue;
			}

			if (modifiedComponent->MatchesType(ComponentData[i]))
			{
				// TODO JAMES: Error
				UE_LOG(ArgusGameLog, Error, TEXT("[%s] Found duplicate component type when assigning to an ArgusEntityTemplate. An ArgusEntity can only have one instance of a component type."), ARGUS_FUNCNAME);
				ComponentData[arrayIndex] = nullptr;
				return;
			}
		}
	}
}
#endif