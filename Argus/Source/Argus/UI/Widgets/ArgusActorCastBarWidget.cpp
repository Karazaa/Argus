// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/ArgusActorCastBarWidget.h"
#include "ArgusEntity.h"
#include "ArgusMath.h"
#include "Components/ProgressBar.h"

void UArgusActorCastBarWidget::SetInitialDisplay(const ArgusEntity& argusEntity)
{
	Super::SetInitialDisplay(argusEntity);

	SetVisibility(ESlateVisibility::Collapsed);
}

void UArgusActorCastBarWidget::RefreshDisplay(const ArgusEntity& argusEntity)
{
	Super::RefreshDisplay(argusEntity);

	ARGUS_RETURN_ON_NULL(m_progressBar, ArgusUILog);

	FLinearColor fillColor = FColor::White;
	float timeElapsedProportion = -1.0f;
	bool shouldBeVisible = false;

	if (const ConstructionComponent* constructionComponent = argusEntity.GetComponent<ConstructionComponent>())
	{
		if (const TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>())
		{
			shouldBeVisible = taskComponent->m_constructionState == EConstructionState::BeingConstructed;
			fillColor = m_constructionProgressColor;
		}

		if (shouldBeVisible)
		{
			timeElapsedProportion = ArgusMath::SafeDivide(constructionComponent->m_currentWorkSeconds, constructionComponent->m_requiredWorkSeconds);
		}
	}

	const SpawningComponent* spawningComponent = argusEntity.GetComponent<SpawningComponent>();
	if (spawningComponent && !shouldBeVisible)
	{
		timeElapsedProportion = spawningComponent->m_spawnTimerHandle.GetTimeElapsedProportion(argusEntity);
		shouldBeVisible = spawningComponent->m_spawnQueue.Num() > 0 || timeElapsedProportion > 0.0f;
		fillColor = m_abilityCastColor;
	}

	shouldBeVisible &= argusEntity.IsAlive();
	const bool isVisible = GetVisibility() != ESlateVisibility::Collapsed;

	if (isVisible)
	{
		if (!shouldBeVisible)
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			m_progressBar->SetPercent(timeElapsedProportion);
		}
	}
	else if (shouldBeVisible)
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
		m_progressBar->SetFillColorAndOpacity(fillColor);
		m_progressBar->SetPercent(timeElapsedProportion);
	}
}
