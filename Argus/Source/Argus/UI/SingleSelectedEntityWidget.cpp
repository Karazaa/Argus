// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SingleSelectedEntityWidget.h"
#include "ArgusActorHealthBarWidget.h"
#include "ArgusEntity.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "AbilityQueueWidget.h"

void USingleSelectedEntityWidget::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	ARGUS_RETURN_ON_NULL(m_entityHealthBar, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_spawnQueue, ArgusUILog);

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent || !inputInterfaceComponent->m_selectedArgusEntityIds.Num())
	{
		return;
	}

	ArgusEntity selectedEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[0]);
	m_entityHealthBar->RefreshDisplay(selectedEntity);
	m_spawnQueue->RefreshDisplay(selectedEntity);
}

void USingleSelectedEntityWidget::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	ARGUS_RETURN_ON_NULL(m_entityImage, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_entityName, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_entityHealthBar, ArgusUILog);

	const UArgusActorRecord* templateArgusActorRecord = templateEntity.GetAssociatedActorRecord();
	if (!templateArgusActorRecord)
	{
		return;
	}

	m_entityImageSlateBrush.SetResourceObject(templateArgusActorRecord->m_actorInfoIcon.LoadAndStorePtr());
	m_entityImage->SetBrush(m_entityImageSlateBrush);
	m_entityName->SetText(templateArgusActorRecord->m_actorInfoName);
	m_entityHealthBar->SetInitialDisplay(templateEntity);
}