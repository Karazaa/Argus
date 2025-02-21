// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIButtonClickedEventsEnum.h"
#include "ArgusUserWidget.h"
#include "SelectedArgusEntitiesWidget.generated.h"

class ArgusEntity;
class UArgusInputManager;

UCLASS()
class USelectedArgusEntitiesWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateSelectedArgusActors(int32 ability0RecordId, int32 ability1RecordId, int32 ability2RecordId, int32 ability3RecordId);

	virtual void OnUpdateSelectedArgusActors(ArgusEntity& templateEntity) override;

protected:
	UFUNCTION(BlueprintCallable)
	void OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent);

	UPROPERTY(EditDefaultsOnly)
	bool m_shouldBlockCameraPanning = true;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};