// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMap.h"
#include "ArgusSetAllocator.h"
#include "ComponentDependencies/EntityCategory.h"
#include "DataComponentDefinitions/ComponentData.h"
#include "Misc/AutomationTest.h"
#include "UObject/SoftObjectPtr.h"
#include "ArgusEntityTemplate.generated.h"

UCLASS()
class ARGUS_API UArgusEntityTemplate : public UDataAsset 
{
	GENERATED_BODY()

public:
	void AsyncLoadComponents(const TFunction<void()> onCompleteCallback = nullptr) const;
	ArgusEntity MakeEntity() const;
	ArgusEntity MakeEntity(uint16 entityId) const;
	ArgusEntity MakeEntityAsync(const TFunction<void(ArgusEntity)> onCompleteCallback = nullptr) const;
	void PopulateEntity(ArgusEntity entity) const;
	void ReinitializeComponentsForEntityPostLoad(ArgusEntity entity) const;
	void SetInitialStateFromData(ArgusEntity entity) const;
	UEntityPriority GetEntityPriority() const { return m_entityPriority; }
	bool DoesTemplateSatisfyEntityCategory(EntityCategory entityCategory) const;

	template <typename ComponentType>
	const ComponentType* GetComponentFromTemplate() const
	{
		ARGUS_TRACE(UArgusEntityTemplate::GetComponentFromTemplate);

		if (const TObjectPtr<const UComponentData>* componentPointer = m_loadedComponentData.Find(ComponentType::StaticClass()))
		{
			if (const ComponentType* component = Cast<ComponentType>(componentPointer->Get()))
			{
				return component;
			}
		}

		return nullptr;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
#endif

private:
	UPROPERTY(EditAnywhere)
	UEntityPriority m_entityPriority;

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<const UComponentData>> m_componentData;

	UPROPERTY(Transient)
	mutable TMap<UClass*, TObjectPtr<const UComponentData>> m_loadedComponentData;

	mutable ArgusMap<EntityCategory, bool, ArgusSetAllocator<14u> > m_isEntityCategorySatisfiedByTemplate;

	void CacheComponents() const;

#if WITH_AUTOMATION_TESTS
	friend class ArgusEntityTemplateInstantiateEntityTest;
	friend class SpawningSystemsSpawnEntityTest;
#endif //WITH_AUTOMATION_TESTS
#if WITH_EDITOR
	friend class UAbilityRecord;
#endif //WITH_EDITOR
};