// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "MarqueeCanvasWidget.h"
#include "ArgusInputManager.h"

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
}