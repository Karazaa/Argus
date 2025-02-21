// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "MarqueeCanvasWidget.generated.h"

class ArgusEntity;
class UArgusInputManager;

UCLASS()
class UMarqueeCanvasWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateFromInputManager(const FVector2D& currentMouseLocation) override;
};