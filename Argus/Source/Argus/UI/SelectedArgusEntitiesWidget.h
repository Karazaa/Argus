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
class UMultipleSelectedEntitiesWidget;
class USingleSelectedEntityWidget;

UCLASS()
class USelectedArgusEntitiesWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateSelectedArgusActorAbilities(const UAbilityRecord* ability0Record, const UAbilityRecord* ability1Record, const UAbilityRecord* ability2Record, const UAbilityRecord* ability3Record);

	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;
	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;

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

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<USingleSelectedEntityWidget> m_singleSelectedEntityWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UMultipleSelectedEntitiesWidget> m_multipleSelectedEntitiesWidget = nullptr;

	FButtonStyle m_abilityButtonStyle;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void HideAllElements();
};