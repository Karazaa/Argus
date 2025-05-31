// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SingleSelectedEntityWidget.h"
#include "ArgusActorHealthBarWidget.h"
#include "ArgusEntity.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RecordDefinitions/ArgusActorRecord.h"

void USingleSelectedEntityWidget::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	if (!m_entityHealthBar)
	{
		// TODO JAMES: Error here
		return;
	}

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent || !inputInterfaceComponent->m_selectedArgusEntityIds.Num())
	{
		return;
	}

	m_entityHealthBar->RefreshDisplay(ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[0]));
}

void USingleSelectedEntityWidget::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	if (!m_entityImage || !m_entityName || !m_entityHealthBar)
	{
		// TODO JAMES: Error here
		return;
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
	m_entityHealthBar->SetInitialDisplay(templateEntity);
}