// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorInfoWidget.h"
#include "ArgusEntity.h"

void UArgusActorInfoWidget::SetInitialInfoState(ArgusEntity& argusEntity)
{
	if (!argusEntity)
	{
		return;
	}

	if (m_castBarWidget)
	{
		m_castBarWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UArgusActorInfoWidget::RefreshInfoDisplay(ArgusEntity& argusEntity)
{
	if (!argusEntity)
	{
		return;
	}

	RefreshCastBarDisplay(argusEntity);
}

void UArgusActorInfoWidget::RefreshCastBarDisplay(ArgusEntity& argusEntity)
{
	if (!m_castBarWidget)
	{
		return;
	}

	const SpawningComponent* spawningComponent = argusEntity.GetComponent<SpawningComponent>();
	if (!spawningComponent)
	{
		return;
	}

	const float timeElapsedProportion = spawningComponent->m_spawnTimerHandle.GetTimeElapsedProportion(argusEntity);
	if (timeElapsedProportion > 0.0f && m_castBarWidget->GetVisibility() == ESlateVisibility::Hidden)
	{
		m_castBarWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else if (timeElapsedProportion <= 0.0f && m_castBarWidget->GetVisibility() != ESlateVisibility::Hidden)
	{
		m_castBarWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
