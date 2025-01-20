// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "ArgusActorInfoWidget.generated.h"

class ArgusEntity;

UCLASS()
class UArgusActorInfoWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	void SetInitialInfoState(ArgusEntity& argusEntity);

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UUserWidget> m_castBarWidget = nullptr;
};