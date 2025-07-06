// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "MultipleSelectedEntitiesView.generated.h"

class UEntityIconWithInfoWidget;
class UImage;
class UUniformGridPanel;
class UUniformGridSlot;

UCLASS()
class UMultipleSelectedEntitiesView : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;
	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UUniformGridPanel> m_uniformGridPanel = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UEntityIconWithInfoWidget> m_entityIconWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	int32 m_numberOfEntitiesPerRow = 8;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_entityImageSlateBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_activeAbilityGroupSlateBrush;

	UPROPERTY(Transient)
	TArray<UEntityIconWithInfoWidget*> m_entityIcons;

	UPROPERTY(Transient)
	TArray<UUniformGridSlot*> m_gridSlots;
};