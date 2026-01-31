// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/IconQueueWidget.h"
#include "ArgusEntity.h"
#include "ArgusInputManager.h"
#include "ArgusStaticData.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Widgets/IconWidget.h"

void UIconQueueWidget::RefreshDisplay(ArgusEntity selectedEntity)
{
	m_trackedEntity = selectedEntity;
	switch (m_iconQueueDataSource)
	{
		case EIconQueueDataSource::SpawnQueue:
			RefreshDisplayFromSpawnQueue();
			break;
		case EIconQueueDataSource::AbilityQueue:
			RefreshDisplayFromAbilityQueue();
			break;
		case EIconQueueDataSource::CarrierPassengers:
			RefreshDisplayFromCarrierPassengers();
			break;
		default:
			break;
	}
}

void UIconQueueWidget::SetIconQueueDataSource(EIconQueueDataSource dataSource, ArgusEntity selectedEntity)
{
	if (dataSource == m_iconQueueDataSource)
	{
		return;
	}

	m_iconQueueDataSource = dataSource;
	RefreshDisplay(selectedEntity);
}

void UIconQueueWidget::OnIconClicked(uint16 identifier)
{
	if (!m_inputManager.IsValid())
	{
		return;
	}

	switch (m_iconQueueDataSource)
	{
		case EIconQueueDataSource::SpawnQueue:
			OnQueuedSpawnIconClicked(identifier);
			break;
		case EIconQueueDataSource::AbilityQueue:
			break;
		case EIconQueueDataSource::CarrierPassengers:
			if (const CarrierComponent* carrierComponent = m_trackedEntity.GetComponent<CarrierComponent>())
			{
				if (identifier < carrierComponent->m_passengerEntityIds.Num())
				{
					m_inputManager->OnUserInterfaceEntityClicked(ArgusEntity::RetrieveEntity(carrierComponent->m_passengerEntityIds[identifier]));
				}
			}
			break;
		default:
			break;
	}
}

void UIconQueueWidget::RefreshDisplayFromSpawnQueue()
{
	const SpawningComponent* spawningComponent = m_trackedEntity.GetComponent<SpawningComponent>();
	if (!spawningComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!IsVisible())
	{
		SetVisibility(ESlateVisibility::Visible);
	}

	TArray<uint32> spawnQueueAbilityRecordIds;
	spawnQueueAbilityRecordIds.Reserve(spawningComponent->m_spawnQueue.Num());

	for (SpawnEntityInfo info : spawningComponent->m_spawnQueue)
	{
		spawnQueueAbilityRecordIds.Add(info.m_spawningAbilityRecordId);
	}

	if (spawnQueueAbilityRecordIds.Num() != m_lastUpdateVisibleIconCount)
	{
		SetIconStates(spawnQueueAbilityRecordIds);
	}
}

void UIconQueueWidget::RefreshDisplayFromAbilityQueue()
{

}

void UIconQueueWidget::RefreshDisplayFromCarrierPassengers()
{
	const CarrierComponent* carrierComponent = m_trackedEntity.GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!IsVisible())
	{
		SetVisibility(ESlateVisibility::Visible);
	}

	const int32 numPassengers = carrierComponent->m_passengerEntityIds.Num();
	TArray<uint32> passengerArgusActorRecordIds;
	passengerArgusActorRecordIds.Reserve(numPassengers);

	for (int32 i = 0; i < numPassengers; ++i)
	{
		ArgusEntity passengerEntity = ArgusEntity::RetrieveEntity(carrierComponent->m_passengerEntityIds[i]);
		const TaskComponent* taskComponent = passengerEntity.GetComponent<TaskComponent>();
		ARGUS_RETURN_ON_NULL(taskComponent, ArgusUILog);
		passengerArgusActorRecordIds.Add(taskComponent->m_spawnedFromArgusActorRecordId);
	}

	if (passengerArgusActorRecordIds.Num() != m_lastUpdateVisibleIconCount)
	{
		SetIconStates(passengerArgusActorRecordIds);
	}
}

void UIconQueueWidget::SetIconStates(const TArray<uint32>& recordIds)
{
	ARGUS_RETURN_ON_NULL(m_uniformGridPanel, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_iconWidgetClass, ArgusUILog);

	const int32 currentNumberOfIcons = m_icons.Num();
	const int32 numberOfRecordsInQueque = recordIds.Num();

	if (numberOfRecordsInQueque == m_lastUpdateVisibleIconCount)
	{
		return;
	}
	m_lastUpdateVisibleIconCount = numberOfRecordsInQueque;

	if (currentNumberOfIcons > numberOfRecordsInQueque)
	{
		for (int32 i = numberOfRecordsInQueque; i < currentNumberOfIcons; ++i)
		{
			if (!m_icons[i])
			{
				continue;
			}

			m_icons[i]->SetVisibility(ESlateVisibility::Collapsed);
			m_gridSlots[i]->SetRow(0);
			m_gridSlots[i]->SetColumn(0);
		}
	}
	else if (numberOfRecordsInQueque > currentNumberOfIcons)
	{
		m_icons.Reserve(numberOfRecordsInQueque);
		m_gridSlots.Reserve(numberOfRecordsInQueque);
		for (int32 i = 0; i < numberOfRecordsInQueque - currentNumberOfIcons; ++i)
		{
			m_icons.Add(CreateWidget<UIconWidget>(GetOwningPlayer(), m_iconWidgetClass));
			const int32 index = m_icons.Num() - 1;
			UIconWidget* icon = m_icons[index];
			ARGUS_RETURN_ON_NULL(icon, ArgusUILog);
			icon->Populate([this](uint16 identifier) { this->OnIconClicked(identifier); }, static_cast<uint16>(index));
			m_gridSlots.Add(m_uniformGridPanel->AddChildToUniformGrid(icon));
		}
	}

	for (int32 i = 0; i < numberOfRecordsInQueque; ++i)
	{
		if (!m_icons[i])
		{
			continue;
		}

		m_icons[i]->SetVisibility(ESlateVisibility::Visible);
		FSlateBrush slotBrush = m_iconImageSlateBrushes[static_cast<uint8>(m_iconQueueDataSource)];
		slotBrush.SetResourceObject(GetIconTextureForRecord(recordIds[i]));
		m_icons[i]->SetBrush(slotBrush);
		m_gridSlots[i]->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		m_gridSlots[i]->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		m_gridSlots[i]->SetRow(0);
		m_gridSlots[i]->SetColumn(i);
	}
}

UTexture* UIconQueueWidget::GetIconTextureForRecord(uint32 recordId)
{
	switch (m_iconQueueDataSource)
	{
		case EIconQueueDataSource::AbilityQueue:
		case EIconQueueDataSource::SpawnQueue:
			if (const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(recordId))
			{
				return abilityRecord->m_abilityIcon.LoadAndStorePtr();
			}
			break;
		case EIconQueueDataSource::CarrierPassengers:
			if (const UArgusActorRecord* actorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(recordId))
			{
				return actorRecord->m_actorInfoIcon.LoadAndStorePtr();
			}
			break;
		default:
			break;
	}

	return nullptr;
}

void UIconQueueWidget::OnQueuedSpawnIconClicked(uint16 queueIndex)
{
	SpawningComponent* spawningComponent = m_trackedEntity.GetComponent<SpawningComponent>();
	ARGUS_RETURN_ON_NULL(spawningComponent, ArgusUILog);

	if (queueIndex + 1 > spawningComponent->m_spawnQueue.Num())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Tried to click a queued spawn icon index that is out of range!"), ARGUS_FUNCNAME);
		return;
	}

	spawningComponent->m_spawnQueueIndexToCancel = queueIndex;
}
