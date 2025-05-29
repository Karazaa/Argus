// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SingleSelectedEntityWidget.h"
#include "ArgusEntity.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RecordDefinitions/ArgusActorRecord.h"


void USingleSelectedEntityWidget::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	const UArgusActorRecord* templateArgusActorRecord = templateEntity.GetAssociatedActorRecord();
	if (!templateArgusActorRecord)
	{
		return;
	}
}