// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/IconQueueWidget.h"
#include "ArgusEntity.h"
#include "ArgusStaticData.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UIconQueueWidget::RefreshDisplay(const ArgusEntity& selectedEntity)
{
	switch (m_iconQueueDataSource)
	{
		case EIconQueueDataSource::SpawnQueue:
			RefreshDisplayFromSpawnQueue(selectedEntity);
			break;
		case EIconQueueDataSource::AbilityQueue:
			RefreshDisplayFromAbilityQueue(selectedEntity);
			break;
		case EIconQueueDataSource::CarrierPassengers:
			RefreshDisplayFromCarrierPassengers(selectedEntity);
			break;
		default:
			break;
	}
}

void UIconQueueWidget::RefreshDisplayFromSpawnQueue(const ArgusEntity& selectedEntity)
{
	const SpawningComponent* spawningComponent = selectedEntity.GetComponent<SpawningComponent>();
	if (!spawningComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!IsVisible())
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	TArray<uint32> spawnQueueAbilityRecordIds;
	spawnQueueAbilityRecordIds.Reserve(spawningComponent->m_currentQueueSize);

	for (SpawnEntityInfo info : spawningComponent->m_spawnQueue)
	{
		spawnQueueAbilityRecordIds.Add(info.m_spawningAbilityRecordId);
	}

	if (spawnQueueAbilityRecordIds.Num() != m_lastUpdateVisibleIconCount)
	{
		SetIconStates(spawnQueueAbilityRecordIds);
	}
}

void UIconQueueWidget::RefreshDisplayFromAbilityQueue(const ArgusEntity& selectedEntity)
{

}

void UIconQueueWidget::RefreshDisplayFromCarrierPassengers(const ArgusEntity& selectedEntity)
{
	const CarrierComponent* carrierComponent = selectedEntity.GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!IsVisible())
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
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
			m_icons.Add(WidgetTree->ConstructWidget<UImage>());
			m_gridSlots.Add(m_uniformGridPanel->AddChildToUniformGrid(m_icons[m_icons.Num() - 1]));
		}
	}

	for (int32 i = 0; i < numberOfRecordsInQueque; ++i)
	{
		if (!m_icons[i])
		{
			continue;
		}

		m_icons[i]->SetVisibility(ESlateVisibility::HitTestInvisible);
		FSlateBrush slotBrush = m_iconImageSlateBrush;
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
