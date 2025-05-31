// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "MultipleSelectedEntitiesWidget.h"
#include "ArgusEntity.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "RecordDefinitions/ArgusActorRecord.h"

void UMultipleSelectedEntitiesWidget::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{

}

void UMultipleSelectedEntitiesWidget::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	if (!m_uniformGridPanel)
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	const InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return;
	}

	const int32 currentNumberOfImages = m_slotImages.Num();
	const int32 numberOfSelectedEntities = inputInterfaceComponent->m_selectedArgusEntityIds.Num();

	if (currentNumberOfImages > numberOfSelectedEntities)
	{
		for (int32 i = numberOfSelectedEntities; i < currentNumberOfImages; ++i)
		{
			if (!m_slotImages[i])
			{
				continue;
			}

			m_slotImages[i]->SetVisibility(ESlateVisibility::Collapsed);
			m_gridSlots[i]->SetRow(0);
			m_gridSlots[i]->SetColumn(0);
		}
	}
	else if (numberOfSelectedEntities > currentNumberOfImages)
	{
		m_slotImages.Reserve(numberOfSelectedEntities);
		m_gridSlots.Reserve(numberOfSelectedEntities);
		for (int32 i = 0; i < numberOfSelectedEntities - currentNumberOfImages; ++i)
		{
			m_slotImages.Add(WidgetTree->ConstructWidget<UImage>());
			m_gridSlots.Add(m_uniformGridPanel->AddChildToUniformGrid(m_slotImages[m_slotImages.Num() - 1]));
		}
	}

	for (int32 i = 0; i < numberOfSelectedEntities; ++i)
	{
		if (!m_slotImages[i])
		{
			continue;
		}

		if (inputInterfaceComponent->m_selectedArgusEntityIds[i] == ArgusECSConstants::k_maxEntities)
		{
			continue;
		}

		const UArgusActorRecord* argusActorRecord = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]).GetAssociatedActorRecord();
		if (!argusActorRecord || argusActorRecord->m_actorInfoIcon.IsNull())
		{
			continue;
		}

		m_slotImages[i]->SetVisibility(ESlateVisibility::HitTestInvisible);
		FSlateBrush slotBrush = inputInterfaceComponent->IsEntityIdInActiveAbilityGroup(inputInterfaceComponent->m_selectedArgusEntityIds[i]) ? 
																						m_activeAbilityGroupSlateBrush : m_entityImageSlateBrush;

		slotBrush.SetResourceObject(argusActorRecord->m_actorInfoIcon.LoadSynchronous());
		m_slotImages[i]->SetBrush(slotBrush);
		m_gridSlots[i]->SetRow(i / m_numberOfEntitiesPerRow);
		m_gridSlots[i]->SetColumn(i % m_numberOfEntitiesPerRow);
	}
}