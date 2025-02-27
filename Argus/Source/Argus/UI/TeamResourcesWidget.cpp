// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamResourcesWidget.h"
#include "ArgusEntity.h"

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