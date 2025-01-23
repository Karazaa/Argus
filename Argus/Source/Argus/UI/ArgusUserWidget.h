// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIButtonClickedEventsEnum.h"
#include "Blueprint/UserWidget.h"
#include "ArgusUserWidget.generated.h"

class UArgusInputManager;

UCLASS()
class UArgusUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateSelectedArgusActors(int32 ability0RecordId, int32 ability1RecordId, int32 ability2RecordId, int32 ability3RecordId);

	void SetInputManager(UArgusInputManager* inputManager);

protected:
	UFUNCTION(BlueprintCallable)
	void OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent);

	UPROPERTY(EditDefaultsOnly)
	bool m_shouldBlockCameraPanning = true;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	TWeakObjectPtr<UArgusInputManager> m_inputManager = nullptr;
};