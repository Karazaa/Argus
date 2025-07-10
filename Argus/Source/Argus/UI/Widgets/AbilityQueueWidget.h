// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "AbilityQueueWidget.generated.h"

class ArgusEntity;
class UImage;
class UUniformGridPanel;
class UUniformGridSlot;

UENUM(BlueprintType)
enum class EIconQueueDataSource : uint8 
{
	SpawnQueue,
	AbilityQueue,
	CarrierPassengers
};

UCLASS()
class UIconQueueWidget : public UArgusUIElement
{
	GENERATED_BODY()

public:
	void RefreshDisplay(const ArgusEntity& selectedEntity);

protected:
	UPROPERTY(EditAnywhere)
	EIconQueueDataSource m_iconQueueDataSource = EIconQueueDataSource::SpawnQueue;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UUniformGridPanel> m_uniformGridPanel = nullptr;

	UPROPERTY(EditAnywhere)
	FSlateBrush m_iconImageSlateBrush;

	UPROPERTY(EditAnywhere)
	FSlateBrush m_secondRowIconImageSlateBrush;

	UPROPERTY(Transient)
	TArray<UImage*> m_icons;

	UPROPERTY(Transient)
	TArray<UUniformGridSlot*> m_gridSlots;

	void RefreshDisplayFromSpawnQueue(const ArgusEntity& selectedEntity);
	void RefreshDisplayFromAbilityQueue(const ArgusEntity& selectedEntity);
	void RefreshDisplayFromCarrierPassengers(const ArgusEntity& selectedEntity);
	void SetIconStates(const TArray<uint32>& recordIds);

	int32 m_lastUpdateAbilityCount = 0;
};