// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "DataComponentDefinitions/ComponentData.h"

void UArgusEntityTemplate::AsyncLoadComponents(const TFunction<void()> onCompleteCallback) const
{
	AssetLoadingComponent* assetLoadingComponent = ArgusEntity::GetSingletonEntity().GetComponent<AssetLoadingComponent>();
	ARGUS_RETURN_ON_NULL(assetLoadingComponent, ArgusECSLog);

	TArray<FSoftObjectPath> pathsToLoad;
	pathsToLoad.Reserve(m_componentData.Num());
	for (int32 i = 0; i < m_componentData.Num(); ++i)
	{
		pathsToLoad.Add(m_componentData[i].ToSoftObjectPath());
	}

	assetLoadingComponent->m_streamableManager.RequestAsyncLoad(pathsToLoad, FStreamableDelegate::CreateLambda
	(
		[this, onCompleteCallback]()
		{
			this->CacheComponents();
			if (onCompleteCallback)
			{
				onCompleteCallback();
			}
		})
	);
}

ArgusEntity UArgusEntityTemplate::MakeEntity() const
{
	ArgusEntity entity = ArgusEntity::CreateEntity(static_cast<uint16>(m_entityPriority));
	CacheComponents();
	PopulateEntity(entity);
	return entity;
}

ArgusEntity UArgusEntityTemplate::MakeEntity(uint16 entityId) const
{
	ArgusEntity entity = ArgusEntity::CreateEntity(entityId);
	CacheComponents();
	PopulateEntity(entity);
	return entity;
}

ArgusEntity UArgusEntityTemplate::MakeEntityAsync(const TFunction<void(ArgusEntity)> onCompleteCallback) const
{
	ArgusEntity entity = ArgusEntity::CreateEntity(static_cast<uint16>(m_entityPriority));

	AsyncLoadComponents([this, entity, onCompleteCallback]()
	{
		this->PopulateEntity(entity);
		if (onCompleteCallback)
		{
			onCompleteCallback(entity);
		}
	});

	return entity;
}

void UArgusEntityTemplate::PopulateEntity(ArgusEntity entity) const
{
	ARGUS_MEMORY_TRACE(ArgusComponentData);

	for (int32 i = 0; i < m_loadedComponentData.Num(); ++i)
	{
		const UComponentData* componentData = m_loadedComponentData[i];
		if (!componentData)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to populate an entity from a template that has a deleted component."), ARGUS_FUNCNAME);
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

void UArgusEntityTemplate::CacheComponents() const
{
	const int32 initialLoadedSize = m_loadedComponentData.Num();
	m_loadedComponentData.Reserve(m_componentData.Num());
	for (int32 i = 0; i < m_componentData.Num(); ++i)
	{
		if (i < initialLoadedSize)
		{
			if (!m_loadedComponentData[i])
			{
				m_loadedComponentData[i] = m_componentData[i].LoadSynchronous();
			}	
		}
		else
		{
			m_loadedComponentData.Add(m_componentData[i].LoadSynchronous());
		}

		if (!m_loadedComponentData[i])
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Did not successfully load component data from soft pointers when loading all components."), ARGUS_FUNCNAME);
			continue;
		}

		m_loadedComponentData[i]->OnComponentDataLoaded();
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