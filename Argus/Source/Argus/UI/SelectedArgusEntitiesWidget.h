// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIButtonClickedEventsEnum.h"
#include "ArgusUserWidget.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "SelectedArgusEntitiesWidget.generated.h"

class ArgusEntity;
class UAbilityRecord;
class UArgusInputManager;
class UButton;

UCLASS()
class USelectedArgusEntitiesWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateSelectedArgusActors(const UAbilityRecord* ability0Record, const UAbilityRecord* ability1Record, const UAbilityRecord* ability2Record, const UAbilityRecord* ability3Record);

	virtual void OnUpdateSelectedArgusActors(ArgusEntity& templateEntity) override;

protected:
	UFUNCTION(BlueprintCallable)
	void UpdateAbilityButtonDisplay(UButton* button, const UAbilityRecord* abilityRecord);

	UFUNCTION(BlueprintCallable)
	void OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent);

	UPROPERTY(EditDefaultsOnly)
	bool m_shouldBlockCameraPanning = true;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityButtonNormalSlateBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityButtonHoveredSlateBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityButtonPressedSlateBrush;

	FButtonStyle m_abilityButtonStyle;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};