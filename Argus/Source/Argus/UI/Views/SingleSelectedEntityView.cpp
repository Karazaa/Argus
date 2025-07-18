// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Views/SingleSelectedEntityView.h"
#include "ArgusEntity.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "Widgets/ArgusActorCastBarWidget.h"
#include "Widgets/ArgusActorHealthBarWidget.h"
#include "Widgets/IconQueueWidget.h"

void USingleSelectedEntityView::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	ARGUS_RETURN_ON_NULL(m_entityHealthBar, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_entityCastBar, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_iconQueue, ArgusUILog);

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent || !inputInterfaceComponent->m_selectedArgusEntityIds.Num())
	{
		return;
	}

	ArgusEntity selectedEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[0]);
	m_entityHealthBar->RefreshDisplay(selectedEntity);
	m_entityCastBar->RefreshDisplay(selectedEntity);
	m_iconQueue->RefreshDisplay(selectedEntity);
}

void USingleSelectedEntityView::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	ARGUS_RETURN_ON_NULL(m_entityImage, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_entityName, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_entityHealthBar, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_entityCastBar, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_iconQueue, ArgusUILog);

	const UArgusActorRecord* templateArgusActorRecord = templateEntity.GetAssociatedActorRecord();
	if (!templateArgusActorRecord)
	{
		return;
	}

	m_entityImageSlateBrush.SetResourceObject(templateArgusActorRecord->m_actorInfoIcon.LoadAndStorePtr());
	m_entityImage->SetBrush(m_entityImageSlateBrush);
	m_entityName->SetText(templateArgusActorRecord->m_actorInfoName);
	m_entityHealthBar->SetInitialDisplay(templateEntity);
	m_entityCastBar->SetInitialDisplay(templateEntity);

	EIconQueueDataSource dataSource = EIconQueueDataSource::AbilityQueue;
	if (CarrierComponent* carrierComponent = templateEntity.GetComponent<CarrierComponent>())
	{
		dataSource = EIconQueueDataSource::CarrierPassengers;
	}
	else if (SpawningComponent* spawningComponent = templateEntity.GetComponent<SpawningComponent>())
	{
		dataSource = EIconQueueDataSource::SpawnQueue;
	}
	m_iconQueue->SetIconQueueDataSource(dataSource, templateEntity);
	m_iconQueue->SetInputManager(m_inputManager.Get());
}