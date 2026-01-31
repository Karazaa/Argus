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

ArgusEntity UArgusEntityTemplate::MakeEntity(uint16 entityId) const
{
	ArgusEntity entity = ArgusEntity::CreateEntity(entityId);
	PopulateEntity(entity);
	return entity;
}

ArgusEntity UArgusEntityTemplate::MakeEntityAsync(const TFunction<void(ArgusEntity)> onCompleteCallback) const
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
		[this, entity, onCompleteCallback]()
		{
			this->PopulateEntity(entity);
			if (onCompleteCallback)
			{
				onCompleteCallback(entity);
			}
		})
	);

	return entity;
}

void UArgusEntityTemplate::PopulateEntity(ArgusEntity entity) const
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

void UArgusEntityTemplate::SetInitialStateFromData(ArgusEntity entity) const
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
		const uint32 ability0 = abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability0);
		const uint32 ability1 = abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability1);
		const uint32 ability2 = abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability2);
		const uint32 ability3 = abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability3);
		if (ability0 > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(ability0);
		}
		if (ability1 > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(ability1);
		}
		if (ability2 > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(ability2);
		}
		if (ability3 > 0u)
		{
			ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(ability3);
		}
	}

	if (const ResourceComponent* resourceComponent = entity.GetComponent<ResourceComponent>())
	{
		ArgusStaticData::AsyncPreLoadRecord<UResourceSetRecord>(resourceComponent->m_resourceCapacityRecordId);
	}

	if (const ResourceExtractionComponent* resourceExtractionComponent = entity.GetComponent<ResourceExtractionComponent>())
	{
		ArgusStaticData::AsyncPreLoadRecord<UResourceSetRecord>(resourceExtractionComponent->m_resourcesToExtractRecordId);
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