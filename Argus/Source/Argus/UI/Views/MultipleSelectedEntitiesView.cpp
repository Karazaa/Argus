// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Views/MultipleSelectedEntitiesView.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "Widgets/EntityIconWithInfoWidget.h"

void UMultipleSelectedEntitiesView::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusUILog);

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

void UMultipleSelectedEntitiesView::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	ARGUS_RETURN_ON_NULL(m_uniformGridPanel, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_entityIconWidgetClass, ArgusUILog);

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
		if (!argusActorRecord)
		{
			continue;
		}

		m_entityIcons[i]->SetVisibility(ESlateVisibility::Visible);
		FSlateBrush slotBrush = inputInterfaceComponent->IsEntityIdInActiveAbilityGroup(inputInterfaceComponent->m_selectedArgusEntityIds[i]) ? 
																						m_activeAbilityGroupSlateBrush : m_entityImageSlateBrush;

		slotBrush.SetResourceObject(argusActorRecord->m_actorInfoIcon.LoadAndStorePtr());
		m_entityIcons[i]->Populate(entity, slotBrush, m_inputManager.Get());
		m_gridSlots[i]->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		m_gridSlots[i]->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		m_gridSlots[i]->SetRow(i / m_numberOfEntitiesPerRow);
		m_gridSlots[i]->SetColumn(i % m_numberOfEntitiesPerRow);
	}
}