// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusUIElement.h"
#include "ComponentObservers/AbilityComponentObservers.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "SelectedArgusEntitiesView.generated.h"

class ArgusEntity;
class UAbilityRecord;
class UArgusInputManager;
class UButton;
class UMultipleSelectedEntitiesView;
class USingleSelectedEntityView;

struct ButtonRecordSet
{
	const UAbilityRecord* m_ability0Record = nullptr;
	const UAbilityRecord* m_ability1Record = nullptr;
	const UAbilityRecord* m_ability2Record = nullptr;
	const UAbilityRecord* m_ability3Record = nullptr;

	ButtonRecordSet() = default;
	ButtonRecordSet(const AbilityComponent* abilityComponent);
};

UCLASS()
class USelectedArgusEntitiesView : public UArgusUIElement, public IAbilityComponentObserver, public IResourceComponentObserver
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;
	virtual void OnUpdateSelectedArgusActors(ArgusEntity templateEntity) override;
	virtual void OnChanged_m_abilityOverrideBitmask(const uint8& oldValue, const uint8& newValue) override;
	virtual void OnChanged_m_currentResources(const FResourceSet& oldValue, const FResourceSet& newValue) override;

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

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityButtonCantAffordSlateBrush;

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

	void UpdateAllAbilityButtonsDisplay(const ButtonRecordSet& buttonRecordSet, const FResourceSet* teamResourceSet = nullptr);
	void UpdateAbilityButtonDisplay(UButton* button, const UAbilityRecord* abilityRecord, const FResourceSet* teamResourceSet);
	void HideAllElements();
	void RemoveTemplateEntityObserver();

	FButtonStyle m_abilityButtonStyle;
	uint16 m_templateEntityId = ArgusECSConstants::k_maxEntities;
};