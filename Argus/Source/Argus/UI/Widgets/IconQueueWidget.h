// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "IconQueueWidget.generated.h"

class ArgusEntity;
class UIconWidget;
class UImage;
class UTexture;
class UUniformGridPanel;
class UUniformGridSlot;

UENUM(BlueprintType)
enum class EIconQueueDataSource : uint8 
{
	SpawnQueue,
	AbilityQueue,
	CarrierPassengers,
	Count UMETA(Hidden)
};

UCLASS()
class UIconQueueWidget : public UArgusUIElement
{
	GENERATED_BODY()

public:
	void RefreshDisplay(const ArgusEntity& selectedEntity);
	void SetIconQueueDataSource(EIconQueueDataSource dataSource, const ArgusEntity& selectedEntity);

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UUniformGridPanel> m_uniformGridPanel = nullptr;

	UPROPERTY(EditAnywhere)
	FSlateBrush m_iconImageSlateBrushes[static_cast<uint8>(EIconQueueDataSource::Count)];

	UPROPERTY(EditAnywhere)
	TSubclassOf<UIconWidget> m_iconWidgetClass = nullptr;

	UPROPERTY(Transient)
	TArray<UIconWidget*> m_icons;

	UPROPERTY(Transient)
	TArray<UUniformGridSlot*> m_gridSlots;

	void RefreshDisplayFromSpawnQueue(const ArgusEntity& selectedEntity);
	void RefreshDisplayFromAbilityQueue(const ArgusEntity& selectedEntity);
	void RefreshDisplayFromCarrierPassengers(const ArgusEntity& selectedEntity);
	void SetIconStates(const TArray<uint32>& recordIds);
	UTexture* GetIconTextureForRecord(uint32 recordId);

	EIconQueueDataSource m_iconQueueDataSource = EIconQueueDataSource::SpawnQueue;
	int32 m_lastUpdateVisibleIconCount = 0;
};