// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ArgusActorInfoView.generated.h"

class ArgusEntity;

UCLASS()
class UArgusActorInfoView : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(ArgusEntity argusEntity);
	virtual void RefreshDisplay(ArgusEntity argusEntity);

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TArray<UArgusActorInfoView*> m_childArgusActorWidgets;
};