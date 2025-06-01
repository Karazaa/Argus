// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamResourcesWidget.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"

void UTeamResourcesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!m_resourceWidgetBar)
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] %s is unset."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_resourceWidgetBar));
		return;
	}

	if (!m_resourceWidgetClass)
	{
		// TODO JAMES: Error here.
		return;
	}

	uint8 numResources = static_cast<uint8>(EResourceType::Count);
	m_resourceWidgetInstances.SetNumUninitialized(numResources);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		m_resourceWidgetInstances[i] = CreateWidget<UResourceWidget>(GetOwningPlayer(), m_resourceWidgetClass);
		if (!m_resourceWidgetInstances[i])
		{
			ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Did not successfully initialize %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UResourceWidget));
			break;
		}

		m_resourceWidgetBar->AddChildToHorizontalBox(m_resourceWidgetInstances[i]);
		m_resourceWidgetInstances[i]->SetPadding(m_resourceWidgetMargin);
	}
}

void UTeamResourcesWidget::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	if (updateDisplayParams.m_team == ETeam::None)
	{
		return;
	}

	ArgusEntity teamEntity = ArgusEntity::GetTeamEntity(updateDisplayParams.m_team);
	if (!teamEntity)
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Did not retrieve a valid %s after calling %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity::GetTeamEntity));
		return;
	}

	ResourceComponent* teamResourceComponent = teamEntity.GetComponent<ResourceComponent>();
	if (!teamResourceComponent)
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[[%s] Did not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ResourceComponent));
		return;
	}

	uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		if (!m_resourceWidgetInstances[i])
		{
			ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] Did not successfully initialize %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UResourceWidget));
			break;
		}
		m_resourceWidgetInstances[i]->UpdateDisplay(static_cast<EResourceType>(i), teamResourceComponent->m_currentResources.m_resourceQuantities[i]);
	}
}