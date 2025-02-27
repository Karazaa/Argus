// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusUserWidget.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void UArgusUserWidget::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	ArgusEntity uiTemplateEntity = ArgusEntity::k_emptyEntity;

	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	if (!m_inputManager->ShouldUpdateSelectedActorDisplay(uiTemplateEntity))
	{
		return;
	}

	OnUpdateSelectedArgusActors(uiTemplateEntity);
}

void UArgusUserWidget::OnUpdateSelectedArgusActors(ArgusEntity& templateEntity)
{

}

void UArgusUserWidget::SetInputManager(UArgusInputManager* inputManager)
{
	m_inputManager = inputManager;
}
