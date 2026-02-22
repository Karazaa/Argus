// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/ArgusActorCastBarWidget.h"
#include "ArgusEntity.h"
#include "ArgusMath.h"
#include "Components/ProgressBar.h"
#include "Systems/FogOfWarSystems.h"

void UArgusActorCastBarWidget::SetInitialDisplay(ArgusEntity argusEntity)
{
	Super::SetInitialDisplay(argusEntity);

	SetVisibility(ESlateVisibility::Collapsed);
}

void UArgusActorCastBarWidget::RefreshDisplay(ArgusEntity entity)
{
	ARGUS_TRACE(UArgusActorCastBarWidget::RefreshDisplay);

	Super::RefreshDisplay(entity);
	ARGUS_RETURN_ON_NULL(m_progressBar, ArgusUILog);

	const bool isVisible = GetVisibility() != ESlateVisibility::Collapsed;
	if (!entity.IsAlive() || (!entity.IsOnPlayerTeam() && FogOfWarSystems::IsFogOfWarVisible()))
	{
		if (isVisible)
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	FLinearColor fillColor = FColor::White;
	float timeElapsedProportion = -1.0f;
	bool shouldBeVisible = false;

	if (const ConstructionComponent* constructionComponent = entity.GetComponent<ConstructionComponent>())
	{
		if (const TaskComponent* taskComponent = entity.GetComponent<TaskComponent>())
		{
			shouldBeVisible = taskComponent->m_constructionState == EConstructionState::BeingConstructed;
			fillColor = m_constructionProgressColor;
		}

		if (shouldBeVisible)
		{
			timeElapsedProportion = ArgusMath::SafeDivide(constructionComponent->m_currentWorkSeconds, constructionComponent->m_requiredWorkSeconds);
		}
	}

	const SpawningComponent* spawningComponent = entity.GetComponent<SpawningComponent>();
	if (spawningComponent && !shouldBeVisible)
	{
		timeElapsedProportion = spawningComponent->m_spawnTimerHandle.GetTimeElapsedProportion(entity);
		shouldBeVisible = spawningComponent->m_spawnQueue.Num() > 0 || timeElapsedProportion > 0.0f;
		fillColor = m_abilityCastColor;
	}

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
