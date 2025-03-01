// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActorWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "ArgusActorInfoWidget.generated.h"

class ArgusEntity;

UCLASS()
class UArgusActorInfoWidget : public UUserWidget, public IArgusActorWidgetInterface
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(ArgusEntity& argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TArray<TObjectPtr<UArgusActorInfoWidget>> m_childArgusActorWidgets;
};