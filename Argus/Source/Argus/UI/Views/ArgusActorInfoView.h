// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActorWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "ArgusActorInfoView.generated.h"

class ArgusEntity;

UCLASS()
class UArgusActorInfoView : public UUserWidget, public IArgusActorWidgetInterface
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(const ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(const ArgusEntity& argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TArray<UArgusActorInfoView*> m_childArgusActorWidgets;
};