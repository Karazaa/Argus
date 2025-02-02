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

	FLinearColor fillColor = FColor::White;
	float timeElapsedProportion = -1.0f;
	bool shouldBeVisible = false;

	if (const ConstructionComponent* constructionComponent = argusEntity.GetComponent<ConstructionComponent>())
	{
		if (const TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>())
		{
			shouldBeVisible = taskComponent->m_constructionState == ConstructionState::BeingConstructed;
			fillColor = m_constructionProgressColor;
		}

		if (shouldBeVisible)
		{
			const float denominator = constructionComponent->m_requiredWorkSeconds > 0.0f ? constructionComponent->m_requiredWorkSeconds : 1.0f;
			timeElapsedProportion = constructionComponent->m_currentWorkSeconds / denominator;
		}
	}

	const SpawningComponent* spawningComponent = argusEntity.GetComponent<SpawningComponent>();
	if (spawningComponent && !shouldBeVisible)
	{
		timeElapsedProportion = spawningComponent->m_spawnTimerHandle.GetTimeElapsedProportion(argusEntity);
		shouldBeVisible = timeElapsedProportion > 0.0f;
		fillColor = m_abilityCastColor;
	}

	const bool isVisible = GetVisibility() != ESlateVisibility::Hidden;

	if (isVisible)
	{
		if (!shouldBeVisible)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
		else if (m_progressBar)
		{
			m_progressBar->SetPercent(timeElapsedProportion);
		}
	}
	else if (shouldBeVisible && m_progressBar)
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
		m_progressBar->SetFillColorAndOpacity(fillColor);
		m_progressBar->SetPercent(timeElapsedProportion);
	}
}
