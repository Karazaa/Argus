// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActorInfoWidget.h"
#include "ArgusActorCarrierBarWidget.generated.h"

class ArgusEntity;
class UProgressBar;

UCLASS()
class UArgusActorCarrierBarWidget : public UArgusActorInfoWidget
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(const ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(const ArgusEntity& argusEntity) override;
};