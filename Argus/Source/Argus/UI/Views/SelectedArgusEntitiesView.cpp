// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Views/SelectedArgusEntitiesView.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "Components/Button.h"
#include "RecordDefinitions/AbilityRecord.h"
#include "Views/MultipleSelectedEntitiesView.h"
#include "Views/SingleSelectedEntityView.h"

ButtonRecordSet::ButtonRecordSet(const AbilityComponent* abilityComponent)
{
	if (!abilityComponent)
	{
		return;
	}

	m_ability0Record = ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability0));
	m_ability1Record = ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability1));
	m_ability2Record = ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability2));
	m_ability3Record = ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability3));
}

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
	m_singleSelectedEntityWidget->SetInputManager(m_inputManager.Get());
	m_multipleSelectedEntitiesWidget->SetInputManager(m_inputManager.Get());

	ReInitializePostLoad();
}

void USelectedArgusEntitiesView::NativeDestruct()
{
	RemoveTemplateEntityObserver();

	ObserversComponent* playerTeamObserversComponent = ArgusEntity::GetPlayerTeamEntity().GetComponent<ObserversComponent>();
	ARGUS_RETURN_ON_NULL(playerTeamObserversComponent, ArgusUILog);

	playerTeamObserversComponent->m_ResourceComponentObservers.RemoveObserver(this);
}

void USelectedArgusEntitiesView::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	Super::UpdateDisplay(updateDisplayParams);

	ARGUS_RETURN_ON_NULL(m_singleSelectedEntityWidget, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_multipleSelectedEntitiesWidget, ArgusUILog);

	if (m_singleSelectedEntityWidget->IsVisible())
	{
		m_singleSelectedEntityWidget->UpdateDisplay(updateDisplayParams);
	}
	else if (m_multipleSelectedEntitiesWidget->IsVisible())
	{
		m_multipleSelectedEntitiesWidget->UpdateDisplay(updateDisplayParams);
	}
}

void USelectedArgusEntitiesView::OnUpdateSelectedArgusActors(ArgusEntity templateEntity)
{
	Super::OnUpdateSelectedArgusActors(templateEntity);

	ARGUS_RETURN_ON_NULL(m_singleSelectedEntityWidget, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_multipleSelectedEntitiesWidget, ArgusUILog);

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

	if (m_templateEntityId != templateEntity.GetId())
	{
		RemoveTemplateEntityObserver();
		if (ObserversComponent* observersComponent = templateEntity.GetComponent<ObserversComponent>())
		{
			observersComponent->m_AbilityComponentObservers.AddObserver(this);
		}

		m_templateEntityId = templateEntity.GetId();
	}

	UpdateAllAbilityButtonsDisplay(ButtonRecordSet(templateEntity.GetComponent<AbilityComponent>()));

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

	if (inputInterfaceComponent->m_selectedArgusEntityIds.Num() > 1)
	{
		m_singleSelectedEntityWidget->SetVisibility(ESlateVisibility::Collapsed);
		m_multipleSelectedEntitiesWidget->SetVisibility(ESlateVisibility::Visible);
		m_multipleSelectedEntitiesWidget->OnUpdateSelectedArgusActors(templateEntity);
	}
	else
	{
		m_singleSelectedEntityWidget->SetVisibility(ESlateVisibility::Visible);
		m_singleSelectedEntityWidget->OnUpdateSelectedArgusActors(templateEntity);
		m_multipleSelectedEntitiesWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USelectedArgusEntitiesView::OnChanged_m_abilityOverrideBitmask(const uint8& oldValue, const uint8& newValue)
{
	if (m_templateEntityId == ArgusECSConstants::k_maxEntities)
	{
		return;
	}
	
	const AbilityComponent* abilityComponent = ArgusEntity::RetrieveEntity(m_templateEntityId).GetComponent<AbilityComponent>();
	if (!abilityComponent)
	{
		return;
	}

	UpdateAllAbilityButtonsDisplay(ButtonRecordSet(abilityComponent));
}

void USelectedArgusEntitiesView::OnChanged_m_currentResources(const FResourceSet& oldValue, const FResourceSet& newValue)
{
	const AbilityComponent* abilityComponent = ArgusEntity::RetrieveEntity(m_templateEntityId).GetComponent<AbilityComponent>();
	if (!abilityComponent)
	{
		return;
	}

	UpdateAllAbilityButtonsDisplay(ButtonRecordSet(abilityComponent), &newValue);
}

void USelectedArgusEntitiesView::ReInitializePostLoad()
{
	ObserversComponent* playerTeamObserversComponent = ArgusEntity::GetPlayerTeamEntity().GetComponent<ObserversComponent>();
	ARGUS_RETURN_ON_NULL(playerTeamObserversComponent, ArgusUILog);

	playerTeamObserversComponent->m_ResourceComponentObservers.AddObserver(this);
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

void USelectedArgusEntitiesView::UpdateAllAbilityButtonsDisplay(const ButtonRecordSet& buttonRecordSet, const FResourceSet* teamResourceSet)
{
	if (!teamResourceSet)
	{
		const ResourceComponent* resourceComponent = ArgusEntity::GetPlayerTeamEntity().GetComponent<ResourceComponent>();
		ARGUS_RETURN_ON_NULL(resourceComponent, ArgusUILog);

		teamResourceSet = &resourceComponent->m_currentResources;
	}

	UpdateAbilityButtonDisplay(m_abilityButton0, buttonRecordSet.m_ability0Record, teamResourceSet);
	UpdateAbilityButtonDisplay(m_abilityButton1, buttonRecordSet.m_ability1Record, teamResourceSet);
	UpdateAbilityButtonDisplay(m_abilityButton2, buttonRecordSet.m_ability2Record, teamResourceSet);
	UpdateAbilityButtonDisplay(m_abilityButton3, buttonRecordSet.m_ability3Record, teamResourceSet);
}

void USelectedArgusEntitiesView::UpdateAbilityButtonDisplay(UButton* button, const UAbilityRecord* abilityRecord, const FResourceSet* teamResourceSet)
{
	ARGUS_RETURN_ON_NULL(button, ArgusUILog);
	ARGUS_RETURN_ON_NULL(teamResourceSet, ArgusUILog);

	if (!abilityRecord)
	{
		button->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	button->SetVisibility(ESlateVisibility::Visible);

	m_abilityButtonNormalSlateBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
	m_abilityButtonHoveredSlateBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
	m_abilityButtonPressedSlateBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());
	m_abilityButtonCantAffordSlateBrush.SetResourceObject(abilityRecord->m_abilityIcon.LoadAndStorePtr());

	if (teamResourceSet->CanAffordResourceChange(abilityRecord->m_requiredResourceChangeToCast))
	{
		m_abilityButtonStyle.SetNormal(m_abilityButtonNormalSlateBrush);
		m_abilityButtonStyle.SetHovered(m_abilityButtonHoveredSlateBrush);
		m_abilityButtonStyle.SetPressed(m_abilityButtonPressedSlateBrush);
	}
	else
	{
		m_abilityButtonStyle.SetNormal(m_abilityButtonCantAffordSlateBrush);
		m_abilityButtonStyle.SetHovered(m_abilityButtonCantAffordSlateBrush);
		m_abilityButtonStyle.SetPressed(m_abilityButtonCantAffordSlateBrush);
	}

	button->SetStyle(m_abilityButtonStyle);
}

void USelectedArgusEntitiesView::HideAllElements()
{
	RemoveTemplateEntityObserver();
	UpdateAllAbilityButtonsDisplay(ButtonRecordSet());

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

void USelectedArgusEntitiesView::RemoveTemplateEntityObserver()
{
	if (m_templateEntityId == ArgusECSConstants::k_maxEntities)
	{
		return;
	}

	ObserversComponent* observersComponent = ArgusEntity::RetrieveEntity(m_templateEntityId).GetComponent<ObserversComponent>();
	if (!observersComponent)
	{
		return;
	}

	observersComponent->m_AbilityComponentObservers.RemoveObserver(this);
}
