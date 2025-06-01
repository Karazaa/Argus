// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "MultipleSelectedEntitiesWidget.h"
#include "ArgusEntity.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "EntityIconWithInfoWidget.h"
#include "RecordDefinitions/ArgusActorRecord.h"

void UMultipleSelectedEntitiesWidget::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		// TODO JAMES: Error here
		return;
	}

	const int32 numSelected = inputInterfaceComponent->m_selectedArgusEntityIds.Num();
	const int32 numIcons = m_entityIcons.Num();
	if (numIcons < numSelected)
	{
		return;
	}

	for (int32 i = 0; i < numSelected; ++i)
	{
		if (!m_entityIcons[i])
		{
			continue;
		}

		m_entityIcons[i]->RefreshDisplay(ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]));
	}
}

void UMultipleSelectedEntitiesWidget::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	if (!m_uniformGridPanel || !m_entityIconWidgetClass)
	{
		// TODO JAMES: Error here.
		return;
	}

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return;
	}

	const int32 currentNumberOfImages = m_entityIcons.Num();
	const int32 numberOfSelectedEntities = inputInterfaceComponent->m_selectedArgusEntityIds.Num();

	if (currentNumberOfImages > numberOfSelectedEntities)
	{
		for (int32 i = numberOfSelectedEntities; i < currentNumberOfImages; ++i)
		{
			if (!m_entityIcons[i])
			{
				continue;
			}

			m_entityIcons[i]->SetVisibility(ESlateVisibility::Collapsed);
			m_gridSlots[i]->SetRow(0);
			m_gridSlots[i]->SetColumn(0);
		}
	}
	else if (numberOfSelectedEntities > currentNumberOfImages)
	{
		m_entityIcons.Reserve(numberOfSelectedEntities);
		m_gridSlots.Reserve(numberOfSelectedEntities);
		for (int32 i = 0; i < numberOfSelectedEntities - currentNumberOfImages; ++i)
		{
			m_entityIcons.Add(CreateWidget<UEntityIconWithInfoWidget>(GetOwningPlayer(), m_entityIconWidgetClass));
			m_gridSlots.Add(m_uniformGridPanel->AddChildToUniformGrid(m_entityIcons[m_entityIcons.Num() - 1]));
		}
	}

	for (int32 i = 0; i < numberOfSelectedEntities; ++i)
	{
		if (!m_entityIcons[i])
		{
			continue;
		}

		if (inputInterfaceComponent->m_selectedArgusEntityIds[i] == ArgusECSConstants::k_maxEntities)
		{
			continue;
		}

		const ArgusEntity entity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
		const UArgusActorRecord* argusActorRecord = entity.GetAssociatedActorRecord();
		if (!argusActorRecord || argusActorRecord->m_actorInfoIcon.IsNull())
		{
			continue;
		}

		m_entityIcons[i]->SetVisibility(ESlateVisibility::HitTestInvisible);
		FSlateBrush slotBrush = inputInterfaceComponent->IsEntityIdInActiveAbilityGroup(inputInterfaceComponent->m_selectedArgusEntityIds[i]) ? 
																						m_activeAbilityGroupSlateBrush : m_entityImageSlateBrush;

		slotBrush.SetResourceObject(argusActorRecord->m_actorInfoIcon.LoadSynchronous());
		m_entityIcons[i]->Populate(entity, slotBrush);
		m_gridSlots[i]->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		m_gridSlots[i]->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		m_gridSlots[i]->SetRow(i / m_numberOfEntitiesPerRow);
		m_gridSlots[i]->SetColumn(i % m_numberOfEntitiesPerRow);
	}
}