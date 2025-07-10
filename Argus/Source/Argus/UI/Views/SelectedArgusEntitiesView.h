// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "SelectedArgusEntitiesView.generated.h"

class ArgusEntity;
class UAbilityRecord;
class UArgusInputManager;
class UButton;
class UMultipleSelectedEntitiesView;
class USingleSelectedEntityView;

UCLASS()
class USelectedArgusEntitiesView : public UArgusUIElement
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;
	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;

protected:
	UFUNCTION()
	void OnClickedAbilityButton0();

	UFUNCTION()
	void OnClickedAbilityButton1();

	UFUNCTION()
	void OnClickedAbilityButton2();

	UFUNCTION()
	void OnClickedAbilityButton3();

	UPROPERTY(EditDefaultsOnly)
	bool m_shouldBlockCameraPanning = true;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityButtonNormalSlateBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityButtonHoveredSlateBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityButtonPressedSlateBrush;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<USingleSelectedEntityView> m_singleSelectedEntityWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UMultipleSelectedEntitiesView> m_multipleSelectedEntitiesWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UButton> m_abilityButton0 = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UButton> m_abilityButton1 = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UButton> m_abilityButton2 = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UButton> m_abilityButton3 = nullptr;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void UpdateAllAbilityButtonsDisplay(const UAbilityRecord* ability0Record, const UAbilityRecord* ability1Record, const UAbilityRecord* ability2Record, const UAbilityRecord* ability3Record);
	void UpdateAbilityButtonDisplay(UButton* button, const UAbilityRecord* abilityRecord);
	void HideAllElements();

	FButtonStyle m_abilityButtonStyle;
};