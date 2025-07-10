// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "Layout/Margin.h"
#include "Widgets/ResourceWidget.h"
#include "TeamResourcesView.generated.h"

class UHorizontalBox;

UCLASS()
class UTeamResourcesView : public UArgusUIElement
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;

protected:
	UPROPERTY(EditAnywhere)
	FMargin m_resourceWidgetMargin;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UResourceWidget> m_resourceWidgetClass = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UHorizontalBox> m_resourceWidgetBar = nullptr;

	TArray<TObjectPtr<UResourceWidget>> m_resourceWidgetInstances;
};