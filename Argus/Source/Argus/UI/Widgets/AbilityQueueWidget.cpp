// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/AbilityQueueWidget.h"
#include "ArgusEntity.h"
#include "ArgusStaticData.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UAbilityQueueWidget::RefreshDisplay(const ArgusEntity& selectedEntity)
{
	switch (m_abilityQueueDataSource)
	{
		case EAbilityQueueDataSource::SpawnQueue:
			RefreshDisplayFromSpawnQueue(selectedEntity);
			break;
		case EAbilityQueueDataSource::AbilityQueue:
			RefreshDisplayFromAbilityQueue(selectedEntity);
			break;
	}
}

void UAbilityQueueWidget::RefreshDisplayFromSpawnQueue(const ArgusEntity& selectedEntity)
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
		SetAbilityIcons(spawnQueueAbilityRecordIds);
	}
}

void UAbilityQueueWidget::RefreshDisplayFromAbilityQueue(const ArgusEntity& selectedEntity)
{

}

void UAbilityQueueWidget::SetAbilityIcons(const TArray<uint32>& abilityRecordIds)
{
	ARGUS_RETURN_ON_NULL(m_uniformGridPanel, ArgusUILog);

	const int32 currentNumberOfIcons = m_abilityIcons.Num();
	const int32 numberOfAbilitiesInQueue = abilityRecordIds.Num();

	if (numberOfAbilitiesInQueue == m_lastUpdateAbilityCount)
	{
		return;
	}
	m_lastUpdateAbilityCount = numberOfAbilitiesInQueue;

	if (currentNumberOfIcons > numberOfAbilitiesInQueue)
	{
		for (int32 i = numberOfAbilitiesInQueue; i < currentNumberOfIcons; ++i)
		{
			if (!m_abilityIcons[i])
			{
				continue;
			}

			m_abilityIcons[i]->SetVisibility(ESlateVisibility::Collapsed);
			m_gridSlots[i]->SetRow(0);
			m_gridSlots[i]->SetColumn(0);
		}
	}
	else if (numberOfAbilitiesInQueue > currentNumberOfIcons)
	{
		m_abilityIcons.Reserve(numberOfAbilitiesInQueue);
		m_gridSlots.Reserve(numberOfAbilitiesInQueue);
		for (int32 i = 0; i < numberOfAbilitiesInQueue - currentNumberOfIcons; ++i)
		{
			m_abilityIcons.Add(WidgetTree->ConstructWidget<UImage>());
			m_gridSlots.Add(m_uniformGridPanel->AddChildToUniformGrid(m_abilityIcons[m_abilityIcons.Num() - 1]));
		}
	}

	for (int32 i = 0; i < numberOfAbilitiesInQueue; ++i)
	{
		if (!m_abilityIcons[i])
		{
			continue;
		}

		const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(abilityRecordIds[i]);
		if (!abilityRecord)
		{
			continue;
		}

		m_abilityIcons[i]->SetVisibility(ESlateVisibility::HitTestInvisible);
		FSlateBrush slotBrush = m_abilityImageSlateBrush;
		slotBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
		m_abilityIcons[i]->SetBrush(slotBrush);
		m_gridSlots[i]->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		m_gridSlots[i]->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		m_gridSlots[i]->SetRow(0);
		m_gridSlots[i]->SetColumn(i);
	}
}
