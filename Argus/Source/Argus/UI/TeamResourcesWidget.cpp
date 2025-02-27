// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamResourcesWidget.h"
#include "ArgusEntity.h"
#include "Blueprint/WidgetTree.h"

void UTeamResourcesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	uint8 numResources = static_cast<uint8>(EResourceType::Count);
	m_resourceWidgetInstances.SetNumUninitialized(numResources);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		m_resourceWidgetInstances[i] = WidgetTree->ConstructWidget<UResourceWidget>(m_resourceWidgetClass, ARGUS_NAMEOF(m_resourceWidgetClass));
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
		return;
	}

	ResourceComponent* teamResourceComponent = teamEntity.GetComponent<ResourceComponent>();
	if (!teamResourceComponent)
	{
		return;
	}

	// TODO JAMES: Display stuff based on resources.
}