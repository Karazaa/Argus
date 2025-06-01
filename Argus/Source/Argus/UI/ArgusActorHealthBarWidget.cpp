// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorHealthBarWidget.h"
#include "ArgusEntity.h"
#include "ArgusMath.h"
#include "Components/ProgressBar.h"

void UArgusActorHealthBarWidget::SetInitialDisplay(const ArgusEntity& argusEntity)
{
	Super::SetInitialDisplay(argusEntity);

	ARGUS_RETURN_ON_NULL(m_progressBar, ArgusUILog);

	if (const HealthComponent* healthComponent = argusEntity.GetComponent<HealthComponent>())
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
		SetHealthBarPercentForEntity(argusEntity);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UArgusActorHealthBarWidget::RefreshDisplay(const ArgusEntity& argusEntity)
{
	Super::RefreshDisplay(argusEntity);
	SetHealthBarPercentForEntity(argusEntity);
}

void UArgusActorHealthBarWidget::SetHealthBarPercentForEntity(const ArgusEntity& argusEntity)
{
	ARGUS_RETURN_ON_NULL(m_progressBar, ArgusUILog);

	const HealthComponent* healthComponent = argusEntity.GetComponent<HealthComponent>();
	if (!healthComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const float currentHealth = static_cast<float>(healthComponent->m_currentHealth);
	const float maximumHealth = static_cast<float>(healthComponent->m_maximumHealth);
	const float healthBarPortion = ArgusMath::SafeDivide(currentHealth, maximumHealth, 1.0);

	if (healthBarPortion != m_progressBar->GetPercent())
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
		m_progressBar->SetPercent(healthBarPortion);
		m_progressBar->SetFillColorAndOpacity(FMath::Lerp(m_lowHealthBarColor, m_fullHealthBarColor, healthBarPortion));
	}

	if (healthBarPortion <= 0.0f || (healthBarPortion >= 1.0f && !m_showHealthBarAtFullHealth))
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}
