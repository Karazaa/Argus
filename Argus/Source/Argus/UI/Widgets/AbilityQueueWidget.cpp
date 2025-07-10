// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/AbilityQueueWidget.h"
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

	if (spawnQueueAbilityRecordIds.Num() != m_lastUpdateAbilityCount)
	{
		SetIconStates(spawnQueueAbilityRecordIds);
	}
}

void UIconQueueWidget::RefreshDisplayFromAbilityQueue(const ArgusEntity& selectedEntity)
{

}

void UIconQueueWidget::RefreshDisplayFromCarrierPassengers(const ArgusEntity& selectedEntity)
{

}

void UIconQueueWidget::SetIconStates(const TArray<uint32>& recordIds)
{
	ARGUS_RETURN_ON_NULL(m_uniformGridPanel, ArgusUILog);

	const int32 currentNumberOfIcons = m_icons.Num();
	const int32 numberOfAbilitiesInQueue = recordIds.Num();

	if (numberOfAbilitiesInQueue == m_lastUpdateAbilityCount)
	{
		return;
	}
	m_lastUpdateAbilityCount = numberOfAbilitiesInQueue;

	if (currentNumberOfIcons > numberOfAbilitiesInQueue)
	{
		for (int32 i = numberOfAbilitiesInQueue; i < currentNumberOfIcons; ++i)
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
	else if (numberOfAbilitiesInQueue > currentNumberOfIcons)
	{
		m_icons.Reserve(numberOfAbilitiesInQueue);
		m_gridSlots.Reserve(numberOfAbilitiesInQueue);
		for (int32 i = 0; i < numberOfAbilitiesInQueue - currentNumberOfIcons; ++i)
		{
			m_icons.Add(WidgetTree->ConstructWidget<UImage>());
			m_gridSlots.Add(m_uniformGridPanel->AddChildToUniformGrid(m_icons[m_icons.Num() - 1]));
		}
	}

	for (int32 i = 0; i < numberOfAbilitiesInQueue; ++i)
	{
		if (!m_icons[i])
		{
			continue;
		}

		const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(recordIds[i]);
		if (!abilityRecord)
		{
			continue;
		}

		m_icons[i]->SetVisibility(ESlateVisibility::HitTestInvisible);
		FSlateBrush slotBrush = m_iconImageSlateBrush;
		slotBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
		m_icons[i]->SetBrush(slotBrush);
		m_gridSlots[i]->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		m_gridSlots[i]->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		m_gridSlots[i]->SetRow(0);
		m_gridSlots[i]->SetColumn(i);
	}
}
