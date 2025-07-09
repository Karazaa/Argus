// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/EntityIconWithInfoWidget.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "Components/Image.h"

void UEntityIconWithInfoWidget::Populate(const ArgusEntity& entity, const FSlateBrush& brush, UArgusInputManager* inputManager)
{
	ARGUS_RETURN_ON_NULL(m_entityIcon, ArgusUILog);
	ARGUS_RETURN_ON_NULL(inputManager, ArgusUILog);

	m_entityIcon->SetBrush(brush);
	SetInitialDisplay(entity);

	m_trackedEntity = entity;
	m_inputManager = inputManager;
}

FReply UEntityIconWithInfoWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!m_inputManager.IsValid())
	{
		return FReply::Handled();
	}

	m_inputManager->OnUserInterfaceEntityClicked(m_trackedEntity);

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
