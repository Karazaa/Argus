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
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool ShouldDrawMarqueeSelectionBox() const;

	UPROPERTY(BlueprintReadOnly)
	FVector2D m_marqueeBoxScreenSpaceStartLocation = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector2D m_marqueeBoxScreenSpaceEndLocation = FVector2D::ZeroVector;
};