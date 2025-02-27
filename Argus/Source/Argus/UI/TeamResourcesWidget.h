// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "ResourceWidget.h"
#include "TeamResourcesWidget.generated.h"

UCLASS()
class UTeamResourcesWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UResourceWidget> m_resourceWidgetClass = nullptr;

	TArray<TObjectPtr<UResourceWidget>> m_resourceWidgetInstances;
};