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
	TArray<TSoftObjectPtr<const UComponentData>> m_componentData;

	UPROPERTY(Transient)
	mutable TArray<TObjectPtr<const UComponentData>> m_loadedComponentData;

	void AsyncLoadComponents(const TFunction<void()> onCompleteCallback = nullptr) const;
	ArgusEntity MakeEntity() const;
	ArgusEntity MakeEntity(uint16 entityId) const;
	ArgusEntity MakeEntityAsync(const TFunction<void(ArgusEntity)> onCompleteCallback = nullptr) const;
	void PopulateEntity(ArgusEntity entity) const;
	void SetInitialStateFromData(ArgusEntity entity) const;

	template <typename ComponentType>
	const ComponentType* GetComponentFromTemplate() const
	{
		ARGUS_TRACE(UArgusEntityTemplate::GetComponentFromTemplate);

		CacheComponents();
		for (int32 i = 0; i < m_loadedComponentData.Num(); ++i)
		{
			const ComponentType* component = Cast<ComponentType>(m_loadedComponentData[i]);
			if (component)
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
	void CacheComponents() const;
};