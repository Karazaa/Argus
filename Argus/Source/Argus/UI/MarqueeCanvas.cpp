// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "MarqueeCanvas.h"
#include "ArgusInputManager.h"
#include "ArgusMath.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"

void UMarqueeCanvas::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	Super::UpdateDisplay(updateDisplayParams);

	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	if (!m_inputManager->ShouldDrawMarqueeBox())
	{
		return;
	}

	const FVector2D& selectionStartScreenSpaceLocation = m_inputManager->GetSelectionStartScreenSpaceLocation();
	const float dpiScale = UWidgetLayoutLibrary::GetViewportScale(this);

	m_marqueeBoxPoints[0] = ArgusMath::SafeDivide(selectionStartScreenSpaceLocation, dpiScale);
	m_marqueeBoxPoints[2] = ArgusMath::SafeDivide(updateDisplayParams.m_currentMouseLocation, dpiScale);
	m_marqueeBoxPoints[4] = m_marqueeBoxPoints[0];
	m_marqueeBoxPoints[1].X = m_marqueeBoxPoints[2].X;
	m_marqueeBoxPoints[1].Y = m_marqueeBoxPoints[0].Y;
	m_marqueeBoxPoints[3].X = m_marqueeBoxPoints[0].X;
	m_marqueeBoxPoints[3].Y = m_marqueeBoxPoints[2].Y;
}

void UMarqueeCanvas::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	m_marqueeBoxPoints.SetNumZeroed(5);
}

int32 UMarqueeCanvas::NativePaint(const FPaintArgs& args, const FGeometry& allottedGeometry, const FSlateRect& myCullingRect, FSlateWindowElementList& outDrawElements, int32 layerId, const FWidgetStyle& inWidgetStyle, bool parentEnabled) const
{
	int32 newLayerId = Super::NativePaint(args, allottedGeometry, myCullingRect, outDrawElements, layerId, inWidgetStyle, parentEnabled);

	// Not erroring here since NativePaint can be called at edit time and not runtime.
	if (!m_inputManager.IsValid())
	{
		return newLayerId;
	}

	if (!m_inputManager->ShouldDrawMarqueeBox())
	{
		return newLayerId;
	}

	FPaintContext context = FPaintContext(allottedGeometry, myCullingRect, outDrawElements, layerId, inWidgetStyle, parentEnabled);
	UWidgetBlueprintLibrary::DrawLines(context, m_marqueeBoxPoints, m_marqueeBoxColor, false, m_marqueeBoxThickness);

	return FMath::Max(newLayerId, context.MaxLayer);
}
