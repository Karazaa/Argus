// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "MarqueeCanvasWidget.h"
#include "ArgusInputManager.h"
#include "ArgusMath.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"

void UMarqueeCanvasWidget::UpdateFromInputManager(const FVector2D& currentMouseLocation)
{
	Super::UpdateFromInputManager(currentMouseLocation);

	if (!m_inputManager.IsValid())
	{
		// TODO JAMES: Error here
		return;
	}

	if (!m_inputManager->ShouldDrawMarqueeBox())
	{
		return;
	}

	const FVector2D& selectionStartScreenSpaceLocation = m_inputManager->GetSelectionStartScreenSpaceLocation();
	const float dpiScale = UWidgetLayoutLibrary::GetViewportScale(this);
	m_marqueeBoxScreenSpaceStartLocation = ArgusMath::SafeDivide(selectionStartScreenSpaceLocation, dpiScale);
	m_marqueeBoxScreenSpaceEndLocation = ArgusMath::SafeDivide(currentMouseLocation, dpiScale);
}

bool UMarqueeCanvasWidget::ShouldDrawMarqueeSelectionBox() const
{
	if (!m_inputManager.IsValid())
	{
		// TODO JAMES: Error here
		return false;
	}

	return m_inputManager->ShouldDrawMarqueeBox();
}