// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "MultipleSelectedEntitiesWidget.h"
#include "ArgusEntity.h"
#include "Components/UniformGridPanel.h"

void UMultipleSelectedEntitiesWidget::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	const UArgusActorRecord* templateArgusActorRecord = templateEntity.GetAssociatedActorRecord();
	if (!templateArgusActorRecord)
	{
		return;
	}
}