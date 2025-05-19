// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIButtonClickedEventsEnum.h"
#include "ArgusUserWidget.h"
#include "SelectedArgusEntitiesWidget.generated.h"

class ArgusEntity;
class UAbilityRecord;
class UArgusInputManager;

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
	void OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent);

	UPROPERTY(EditDefaultsOnly)
	bool m_shouldBlockCameraPanning = true;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};