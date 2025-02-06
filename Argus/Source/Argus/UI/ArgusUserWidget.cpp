// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusUserWidget.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void UArgusUserWidget::OnUpdateSelectedArgusActors(ArgusEntity& templateEntity)
{
	if (!templateEntity)
	{
		OnUpdateSelectedArgusActors(0, 0, 0, 0);
		return;
	}

	if (const TaskComponent* taskComponent = templateEntity.GetComponent<TaskComponent>())
	{
		if (taskComponent->m_constructionState == ConstructionState::BeingConstructed)
		{
			OnUpdateSelectedArgusActors(0, 0, 0, 0);
			return;
		}
	}

	if (const AbilityComponent* abilityComponent = templateEntity.GetComponent<AbilityComponent>())
	{
		OnUpdateSelectedArgusActors(abilityComponent->m_ability0Id, abilityComponent->m_ability1Id, abilityComponent->m_ability2Id, abilityComponent->m_ability3Id);
	}
	else
	{
		OnUpdateSelectedArgusActors(0, 0, 0, 0);
	}
}

void UArgusUserWidget::SetInputManager(UArgusInputManager* inputManager)
{
	m_inputManager = inputManager;
}

void UArgusUserWidget::OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent)
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(buttonClickedEvent);
}

void UArgusUserWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::IncrementPanningBlockers();
	}
}

void UArgusUserWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::DecrementPanningBlockers();
	}
}
