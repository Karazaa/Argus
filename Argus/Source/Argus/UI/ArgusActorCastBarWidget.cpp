// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorCastBarWidget.h"
#include "ArgusEntity.h"
#include "Components/ProgressBar.h"

void UArgusActorCastBarWidget::SetInitialDisplay(ArgusEntity& argusEntity)
{
	Super::SetInitialDisplay(argusEntity);

	SetVisibility(ESlateVisibility::Hidden);
}

void UArgusActorCastBarWidget::RefreshDisplay(ArgusEntity& argusEntity)
{
	Super::RefreshDisplay(argusEntity);

	const SpawningComponent* spawningComponent = argusEntity.GetComponent<SpawningComponent>();
	if (!spawningComponent)
	{
		return;
	}

	const float timeRemainingProportion = spawningComponent->m_spawnTimerHandle.GetTimeRemainingProportion(argusEntity);
	const bool isVisible = GetVisibility() != ESlateVisibility::Hidden;

	if (isVisible)
	{
		if (timeRemainingProportion <= 0.0f)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
		else if (m_progressBar)
		{
			m_progressBar->SetPercent(1.0f - timeRemainingProportion);
		}
	}
	else
	{
		if (timeRemainingProportion > 0.0f)
		{
			SetVisibility(ESlateVisibility::HitTestInvisible);
			m_progressBar->SetPercent(1.0f - timeRemainingProportion);
		}
	}
}
