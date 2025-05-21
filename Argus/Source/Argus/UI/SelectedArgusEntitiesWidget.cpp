// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SelectedArgusEntitiesWidget.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"
#include "Components/Button.h"

void USelectedArgusEntitiesWidget::OnUpdateSelectedArgusActors(ArgusEntity& templateEntity)
{
	Super::OnUpdateSelectedArgusActors(templateEntity);

	if (!templateEntity)
	{
		OnUpdateSelectedArgusActors(nullptr, nullptr, nullptr, nullptr);
		return;
	}

	if (const TaskComponent* taskComponent = templateEntity.GetComponent<TaskComponent>())
	{
		if (taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			OnUpdateSelectedArgusActors(nullptr, nullptr, nullptr, nullptr);
			return;
		}
	}

	if (const AbilityComponent* abilityComponent = templateEntity.GetComponent<AbilityComponent>())
	{
		const UAbilityRecord* ability0Record = abilityComponent->m_ability0Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability0Id);
		const UAbilityRecord* ability1Record = abilityComponent->m_ability1Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability1Id);
		const UAbilityRecord* ability2Record = abilityComponent->m_ability2Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability2Id);
		const UAbilityRecord* ability3Record = abilityComponent->m_ability3Id == 0 ? nullptr : ArgusStaticData::GetRecord<UAbilityRecord>(abilityComponent->m_ability3Id);

		OnUpdateSelectedArgusActors(ability0Record, ability1Record, ability2Record, ability3Record);
	}
	else
	{
		OnUpdateSelectedArgusActors(nullptr, nullptr, nullptr, nullptr);
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
		// TODO JAMES: Show multiple entity display.
	}
	else
	{
		// TODO JAMES: Show single entity display.
	}
}

void USelectedArgusEntitiesWidget::UpdateAbilityButtonDisplay(UButton* button, const UAbilityRecord* abilityRecord)
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

	UTexture* texture = nullptr;
	if (!abilityRecord->m_abilityIcon.IsNull())
	{
		texture = abilityRecord->m_abilityIcon.LoadSynchronous();
	}

	m_abilityButtonNormalSlateBrush.SetResourceObject(texture);
	m_abilityButtonHoveredSlateBrush.SetResourceObject(texture);
	m_abilityButtonPressedSlateBrush.SetResourceObject(texture);
	m_abilityButtonStyle.SetNormal(m_abilityButtonNormalSlateBrush);
	m_abilityButtonStyle.SetHovered(m_abilityButtonHoveredSlateBrush);
	m_abilityButtonStyle.SetPressed(m_abilityButtonPressedSlateBrush);
	button->SetStyle(m_abilityButtonStyle);
}

void USelectedArgusEntitiesWidget::OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent)
{
	if (!m_inputManager.IsValid())
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Invalid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_inputManager));
		return;
	}

	m_inputManager->OnUserInterfaceButtonClicked(buttonClickedEvent);
}

void USelectedArgusEntitiesWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::IncrementPanningBlockers();
	}
}

void USelectedArgusEntitiesWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (m_shouldBlockCameraPanning)
	{
		AArgusCameraActor::DecrementPanningBlockers();
	}
}