// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorInfoWidget.h"
#include "ArgusEntity.h"

void UArgusActorInfoWidget::SetInitialInfoState(ArgusEntity& argusEntity)
{
	if (!argusEntity)
	{
		return;
	}

	AbilityComponent* abilityComponent = argusEntity.GetComponent<AbilityComponent>();
	if (!abilityComponent)
	{
		if (m_castBarWidget)
		{
			m_castBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}
	else
	{
		if (m_castBarWidget)
		{
			m_castBarWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}
