// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "ComponentObservers/AbilityComponentObservers.h"
#include "Layout/Margin.h"
#include "Widgets/ResourceWidget.h"
#include "TeamResourcesView.generated.h"

class UHorizontalBox;

UCLASS()
class UTeamResourcesView : public UArgusUIElement, public IResourceComponentObserver
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnChanged_m_currentResources(const FResourceSet& oldValue, const FResourceSet& newValue) override;

	void UpdateResources(const FResourceSet& newValue);

protected:
	UPROPERTY(EditAnywhere)
	FMargin m_resourceWidgetMargin;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UResourceWidget> m_resourceWidgetClass = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UHorizontalBox> m_resourceWidgetBar = nullptr;

	TArray<TObjectPtr<UResourceWidget>> m_resourceWidgetInstances;
};