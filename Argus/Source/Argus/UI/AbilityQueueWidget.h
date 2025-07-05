// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "AbilityQueueWidget.generated.h"

class ArgusEntity;
class UImage;
class UUniformGridPanel;
class UUniformGridSlot;

UENUM(BlueprintType)
enum class EAbilityQueueDataSource : uint8 
{
	SpawnQueue,
	AbilityQueue
};

UCLASS()
class UAbilityQueueWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	void RefreshDisplay(const ArgusEntity& selectedEntity);

protected:
	UPROPERTY(EditAnywhere)
	EAbilityQueueDataSource m_abilityQueueDataSource = EAbilityQueueDataSource::SpawnQueue;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UUniformGridPanel> m_uniformGridPanel = nullptr;

	UPROPERTY(EditAnywhere)
	int32 m_numberOfAbilitiesPerRow = 8;

	UPROPERTY(EditAnywhere)
	FSlateBrush m_abilityImageSlateBrush;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UImage*> m_abilityIcons;

	UPROPERTY(Transient)
	TArray<UUniformGridSlot*> m_gridSlots;

	void RefreshDisplayFromSpawnQueue(const ArgusEntity& selectedEntity);
	void RefreshDisplayFromAbilityQueue(const ArgusEntity& selectedEntity);
	void SetAbilityIcons(const TArray<uint32>& abilityRecordIds);
};