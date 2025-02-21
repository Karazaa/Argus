// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SelectedArgusEntitiesWidget.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void USelectedArgusEntitiesWidget::OnUpdateSelectedArgusActors(ArgusEntity& templateEntity)
{
	Super::OnUpdateSelectedArgusActors(templateEntity);

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

void USelectedArgusEntitiesWidget::OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent)
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(buttonClickedEvent);
}

void USelectedArgusEntitiesWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::IncrementPanningBlockers();
	}
}

void USelectedArgusEntitiesWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::DecrementPanningBlockers();
	}
}