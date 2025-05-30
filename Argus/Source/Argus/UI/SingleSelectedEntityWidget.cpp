// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SingleSelectedEntityWidget.h"
#include "ArgusEntity.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RecordDefinitions/ArgusActorRecord.h"


void USingleSelectedEntityWidget::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	if (!m_entityImage || !m_entityName)
	{
		// TODO JAMES: Error here
	}

	const UArgusActorRecord* templateArgusActorRecord = templateEntity.GetAssociatedActorRecord();
	if (!templateArgusActorRecord)
	{
		return;
	}

	UTexture* texture = nullptr;
	if (!templateArgusActorRecord->m_actorInfoIcon.IsNull())
	{
		texture = templateArgusActorRecord->m_actorInfoIcon.LoadSynchronous();
	}

	m_entityImageSlateBrush.SetResourceObject(texture);
	m_entityImage->SetBrush(m_entityImageSlateBrush);
	m_entityName->SetText(templateArgusActorRecord->m_actorInfoName);
}