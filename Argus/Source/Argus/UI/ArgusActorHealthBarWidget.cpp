// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorHealthBarWidget.h"
#include "ArgusEntity.h"
#include "ArgusMath.h"
#include "Components/ProgressBar.h"

void UArgusActorHealthBarWidget::SetInitialDisplay(ArgusEntity& argusEntity)
{
	Super::SetInitialDisplay(argusEntity);

	if (!m_progressBar)
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_progressBar));
		return;
	}

	if (const HealthComponent* healthComponent = argusEntity.GetComponent<HealthComponent>())
	{
		SetVisibility(ESlateVisibility::Visible);
		m_progressBar->SetFillColorAndOpacity(m_healthBarColor);
		SetHealthBarPercentForEntity(argusEntity);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UArgusActorHealthBarWidget::RefreshDisplay(ArgusEntity& argusEntity)
{
	Super::RefreshDisplay(argusEntity);
	SetHealthBarPercentForEntity(argusEntity);
}

void UArgusActorHealthBarWidget::SetHealthBarPercentForEntity(ArgusEntity& argusEntity)
{
	if (!m_progressBar)
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_progressBar));
		return;
	}

	const HealthComponent* healthComponent = argusEntity.GetComponent<HealthComponent>();
	if (!healthComponent)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const float currentHealth = static_cast<float>(healthComponent->m_currentHealth);
	const float maximumHealth = static_cast<float>(healthComponent->m_maximumHealth);
	const float healthBarPortion = ArgusMath::SafeDivide(currentHealth, maximumHealth, 1.0);

	if (healthBarPortion != m_progressBar->GetPercent())
	{
		m_progressBar->SetPercent(healthBarPortion);
	}

	if (healthBarPortion >= 1.0f)
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}
