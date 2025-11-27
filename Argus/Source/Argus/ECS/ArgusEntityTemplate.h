// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "DataComponentDefinitions/ComponentData.h"
#include "UObject/SoftObjectPtr.h"
#include "ArgusEntityTemplate.generated.h"

UCLASS()
class UArgusEntityTemplate : public UDataAsset 
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UEntityPriority m_entityPriority;

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UComponentData>> m_componentData;

	ArgusEntity MakeEntity() const;
	ArgusEntity MakeEntity(uint16 entityId) const;
	ArgusEntity MakeEntityAsync(const TFunction<void(ArgusEntity)> onCompleteCallback = nullptr) const;
	void PopulateEntity(const ArgusEntity& entity) const;
	void SetInitialStateFromData(const ArgusEntity& entity) const;

	template <typename ComponentType>
	const ComponentType* GetComponentFromTemplate() const
	{
		for (int32 i = 0; i < m_componentData.Num(); ++i)
		{
			const ComponentType* component = Cast<ComponentType>(m_componentData[i].LoadSynchronous());
			if (component)
			{
				return component;
			}
		}

		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve desired component reference from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UArgusEntityTemplate));
		return nullptr;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
#endif
};