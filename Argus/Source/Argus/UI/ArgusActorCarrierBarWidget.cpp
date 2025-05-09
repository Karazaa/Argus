// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorCarrierBarWidget.h"
#include "ArgusEntity.h"

void UArgusActorCarrierBarWidget::SetInitialDisplay(const ArgusEntity& entity)
{
	Super::SetInitialDisplay(entity);

	SetVisibility(ESlateVisibility::Collapsed);
}

void UArgusActorCarrierBarWidget::RefreshDisplay(const ArgusEntity& entity)
{
	Super::RefreshDisplay(entity);
}