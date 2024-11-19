// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "DataComponentDefinitions/ComponentData.h"

ArgusEntity UArgusEntityTemplate::MakeEntity()
{
	ArgusEntity entity = ArgusEntity::CreateEntity(static_cast<uint16>(m_entityPriority));

	for (TSoftObjectPtr<UComponentData> componentDataSoftObjectPtr : m_componentData)
	{
		UComponentData* componentData = componentDataSoftObjectPtr.LoadSynchronous();
		if (!componentData)
		{
			continue;
		}

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
	const FString componentDataPropertyName = ARGUS_NAMEOF(m_componentData);
	if (!propertyName.Equals(componentDataPropertyName))
	{
		return;
	}

	const int32 arrayIndex = propertyChangedEvent.GetArrayIndex(propertyName);
	const UComponentData* modifiedComponent = m_componentData[arrayIndex].LoadSynchronous();
	if (modifiedComponent)
	{
		for (int i = 0; i < m_componentData.Num(); ++i)
		{
			if (i == arrayIndex)
			{
				continue;
			}

			if (modifiedComponent->MatchesType(m_componentData[i].LoadSynchronous()))
			{
				ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Found duplicate component type when assigning to an ArgusEntityTemplate. An ArgusEntity can only have one instance of a component type."), ARGUS_FUNCNAME);
				m_componentData[arrayIndex] = nullptr;
				return;
			}
		}
	}
}
#endif