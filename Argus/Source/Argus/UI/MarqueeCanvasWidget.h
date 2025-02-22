// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "MarqueeCanvasWidget.generated.h"

class ArgusEntity;
class UArgusInputManager;
class UCanvasPanel;

UCLASS()
class UMarqueeCanvasWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateFromInputManager(const FVector2D& currentMouseLocation) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	FLinearColor m_marqueeBoxColor = FColor::Green;

	UPROPERTY(EditDefaultsOnly)
	float m_marqueeBoxThickness = 1.0f;

	virtual void NativeOnInitialized() override;
	virtual int32 NativePaint(const FPaintArgs& args, const FGeometry& allottedGeometry, const FSlateRect& myCullingRect, FSlateWindowElementList& outDrawElements, int32 layerId, const FWidgetStyle& inWidgetStyle, bool parentEnabled) const override;

	TArray<FVector2D> m_marqueeBoxPoints;
};