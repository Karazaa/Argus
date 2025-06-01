// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "EntityIconWithInfoWidget.h"
#include "Components/Image.h"

void UEntityIconWithInfoWidget::Populate(const ArgusEntity& entity, const FSlateBrush& brush)
{
	if (!m_entityIcon)
	{
		return;
	}

	m_entityIcon->SetBrush(brush);
	SetInitialDisplay(entity);
}