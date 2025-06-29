// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "DataComponentDefinitions/ComponentData.h"

ArgusEntity UArgusEntityTemplate::MakeEntity() const
{
	ArgusEntity entity = ArgusEntity::CreateEntity(static_cast<uint16>(m_entityPriority));
	PopulateEntity(entity);
	return entity;
}

ArgusEntity UArgusEntityTemplate::MakeEntityAsync() const
{
	AssetLoadingComponent* assetLoadingComponent = ArgusEntity::GetSingletonEntity().GetComponent<AssetLoadingComponent>();
	if (!assetLoadingComponent)
	{
		ARGUS_ERROR_NULL(ArgusECSLog, assetLoadingComponent);
		return ArgusEntity::k_emptyEntity;
	}

	ArgusEntity entity = ArgusEntity::CreateEntity(static_cast<uint16>(m_entityPriority));

	TArray<FSoftObjectPath> pathsToLoad;
	pathsToLoad.Reserve(m_componentData.Num());
	for (TSoftObjectPtr<UComponentData> componentDataSoftObjectPtr : m_componentData)
	{
		pathsToLoad.Add(componentDataSoftObjectPtr.ToSoftObjectPath());
	}

	assetLoadingComponent->m_streamableManager.RequestAsyncLoad(pathsToLoad, FStreamableDelegate::CreateLambda
	(
		[this, &entity]()
		{
			this->PopulateEntity(entity);
		})
	);

	return entity;
}

void UArgusEntityTemplate::PopulateEntity(const ArgusEntity& entity) const
{
	ARGUS_MEMORY_TRACE(ArgusComponentData);

	for (TSoftObjectPtr<UComponentData> componentDataSoftObjectPtr : m_componentData)
	{
		UComponentData* componentData = componentDataSoftObjectPtr.LoadSynchronous();
		if (!componentData)
		{
			continue;
		}

		componentData->InstantiateComponentForEntity(entity);
	}

	SetInitialStateFromData(entity);
}

void UArgusEntityTemplate::SetInitialStateFromData(const ArgusEntity& entity) const
{
	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	if (const ConstructionComponent* constructionComponent = entity.GetComponent<ConstructionComponent>())
	{
		if (constructionComponent->m_currentWorkSeconds != 0.0f)
		{
			taskComponent->m_constructionState = EConstructionState::BeingConstructed;
		}
	}

	if (const AbilityComponent* abilityComponent = entity.GetComponent<AbilityComponent>())
	{
		if (abilityComponent->m_ability0Id > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(abilityComponent->m_ability0Id);
		}
		if (abilityComponent->m_ability1Id > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(abilityComponent->m_ability1Id);
		}
		if (abilityComponent->m_ability2Id > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(abilityComponent->m_ability2Id);
		}
		if (abilityComponent->m_ability3Id > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(abilityComponent->m_ability3Id);
		}
	}
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
		for (int32 i = 0; i < m_componentData.Num(); ++i)
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