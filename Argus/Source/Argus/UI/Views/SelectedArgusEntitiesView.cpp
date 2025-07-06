// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Views/SelectedArgusEntitiesView.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "Components/Button.h"
#include "Views/MultipleSelectedEntitiesView.h"
#include "Views/SingleSelectedEntityView.h"

void USelectedArgusEntitiesView::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	Super::UpdateDisplay(updateDisplayParams);

	if (m_singleSelectedEntityWidget && m_singleSelectedEntityWidget->IsVisible())
	{
		m_singleSelectedEntityWidget->UpdateDisplay(updateDisplayParams);
	}
	else if (m_multipleSelectedEntitiesWidget && m_multipleSelectedEntitiesWidget->IsVisible())
	{
		m_multipleSelectedEntitiesWidget->UpdateDisplay(updateDisplayParams);
	}
}

void USelectedArgusEntitiesView::OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity)
{
	Super::OnUpdateSelectedArgusActors(templateEntity);

	if (!templateEntity)
	{
		HideAllElements();
		return;
	}

	if (const TaskComponent* taskComponent = templateEntity.GetComponent<TaskComponent>())
	{
		if (taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			HideAllElements();
			return;
		}
	}

	if (const AbilityComponent* abilityComponent = templateEntity.GetComponent<AbilityComponent>())
	{
		const UAbilityRecord* ability0Record = abilityComponent->m_ability0Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability0Id);
		const UAbilityRecord* ability1Record = abilityComponent->m_ability1Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability1Id);
		const UAbilityRecord* ability2Record = abilityComponent->m_ability2Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability2Id);
		const UAbilityRecord* ability3Record = abilityComponent->m_ability3Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability3Id);

		OnUpdateSelectedArgusActorAbilities(ability0Record, ability1Record, ability2Record, ability3Record);
	}
	else
	{
		OnUpdateSelectedArgusActorAbilities(nullptr, nullptr, nullptr, nullptr);
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return;
	}

	if (inputInterfaceComponent->m_selectedArgusEntityIds.Num() > 1)
	{
		if (m_singleSelectedEntityWidget)
		{
			m_singleSelectedEntityWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (m_multipleSelectedEntitiesWidget)
		{
			m_multipleSelectedEntitiesWidget->SetVisibility(ESlateVisibility::Visible);
			m_multipleSelectedEntitiesWidget->OnUpdateSelectedArgusActors(templateEntity);
		}
	}
	else
	{
		if (m_singleSelectedEntityWidget)
		{
			m_singleSelectedEntityWidget->SetVisibility(ESlateVisibility::Visible);
			m_singleSelectedEntityWidget->OnUpdateSelectedArgusActors(templateEntity);
		}
		if (m_multipleSelectedEntitiesWidget)
		{
			m_multipleSelectedEntitiesWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void USelectedArgusEntitiesView::UpdateAbilityButtonDisplay(UButton* button, const UAbilityRecord* abilityRecord)
{
	if (!button)
	{
		return;
	}

	if (!abilityRecord)
	{
		button->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	button->SetVisibility(ESlateVisibility::Visible);

	m_abilityButtonNormalSlateBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
	m_abilityButtonHoveredSlateBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
	m_abilityButtonPressedSlateBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
	m_abilityButtonStyle.SetNormal(m_abilityButtonNormalSlateBrush);
	m_abilityButtonStyle.SetHovered(m_abilityButtonHoveredSlateBrush);
	m_abilityButtonStyle.SetPressed(m_abilityButtonPressedSlateBrush);
	button->SetStyle(m_abilityButtonStyle);
}

void USelectedArgusEntitiesView::OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent)
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(buttonClickedEvent);
}

void USelectedArgusEntitiesView::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::IncrementPanningBlockers();
	}
}

void USelectedArgusEntitiesView::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::DecrementPanningBlockers();
	}
}

void USelectedArgusEntitiesView::HideAllElements()
{
	OnUpdateSelectedArgusActorAbilities(nullptr, nullptr, nullptr, nullptr);

	if (m_singleSelectedEntityWidget)
	{
		m_singleSelectedEntityWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (m_multipleSelectedEntitiesWidget)
	{
		m_multipleSelectedEntitiesWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}