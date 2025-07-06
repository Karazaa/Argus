// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Views/SelectedArgusEntitiesView.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "Components/Button.h"
#include "Views/MultipleSelectedEntitiesView.h"
#include "Views/SingleSelectedEntityView.h"

void USelectedArgusEntitiesView::NativeConstruct()
{
	Super::NativeConstruct();

	ARGUS_RETURN_ON_NULL(m_abilityButton0, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_abilityButton1, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_abilityButton2, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_abilityButton3, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_singleSelectedEntityWidget, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_multipleSelectedEntitiesWidget, ArgusUILog);
	m_abilityButton0->OnClicked.AddDynamic(this, &USelectedArgusEntitiesView::OnClickedAbilityButton0);
	m_abilityButton1->OnClicked.AddDynamic(this, &USelectedArgusEntitiesView::OnClickedAbilityButton1);
	m_abilityButton2->OnClicked.AddDynamic(this, &USelectedArgusEntitiesView::OnClickedAbilityButton2);
	m_abilityButton3->OnClicked.AddDynamic(this, &USelectedArgusEntitiesView::OnClickedAbilityButton3);
	m_singleSelectedEntityWidget->SetVisibility(ESlateVisibility::Collapsed);
	m_multipleSelectedEntitiesWidget->SetVisibility(ESlateVisibility::Collapsed);
}

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

		UpdateAllAbilityButtonsDisplay(ability0Record, ability1Record, ability2Record, ability3Record);
	}
	else
	{
		UpdateAllAbilityButtonsDisplay(nullptr, nullptr, nullptr, nullptr);
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

void USelectedArgusEntitiesView::UpdateAllAbilityButtonsDisplay(const UAbilityRecord* ability0Record, const UAbilityRecord* ability1Record, const UAbilityRecord* ability2Record, const UAbilityRecord* ability3Record)
{
	UpdateAbilityButtonDisplay(m_abilityButton0, ability0Record);
	UpdateAbilityButtonDisplay(m_abilityButton1, ability1Record);
	UpdateAbilityButtonDisplay(m_abilityButton2, ability2Record);
	UpdateAbilityButtonDisplay(m_abilityButton3, ability3Record);
}

void USelectedArgusEntitiesView::UpdateAbilityButtonDisplay(UButton* button, const UAbilityRecord* abilityRecord)
{
	ARGUS_RETURN_ON_NULL(button, ArgusUILog);

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

void USelectedArgusEntitiesView::HideAllElements()
{
	UpdateAllAbilityButtonsDisplay(nullptr, nullptr, nullptr, nullptr);

	if (m_singleSelectedEntityWidget)
	{
		m_singleSelectedEntityWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (m_multipleSelectedEntitiesWidget)
	{
		m_multipleSelectedEntitiesWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USelectedArgusEntitiesView::OnClickedAbilityButton0()
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(UArgusInputManager::InputType::Ability0);
}

void USelectedArgusEntitiesView::OnClickedAbilityButton1()
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(UArgusInputManager::InputType::Ability1);
}

void USelectedArgusEntitiesView::OnClickedAbilityButton2()
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(UArgusInputManager::InputType::Ability2);
}

void USelectedArgusEntitiesView::OnClickedAbilityButton3()
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(UArgusInputManager::InputType::Ability3);
}
