// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Views/TeamResourcesView.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"

void UTeamResourcesView::NativeConstruct()
{
	Super::NativeConstruct();

	ARGUS_RETURN_ON_NULL(m_resourceWidgetBar, ArgusUILog);
	ARGUS_RETURN_ON_NULL(m_resourceWidgetClass, ArgusUILog);

	uint8 numResources = static_cast<uint8>(EResourceType::Count);
	m_resourceWidgetInstances.SetNumUninitialized(numResources);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		m_resourceWidgetInstances[i] = CreateWidget<UResourceWidget>(GetOwningPlayer(), m_resourceWidgetClass);
		ARGUS_RETURN_ON_NULL(m_resourceWidgetInstances[i], ArgusUILog);

		m_resourceWidgetBar->AddChildToHorizontalBox(m_resourceWidgetInstances[i]);
		m_resourceWidgetInstances[i]->SetPadding(m_resourceWidgetMargin);
	}
}

void UTeamResourcesView::UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams)
{
	if (updateDisplayParams.m_team == ETeam::None)
	{
		return;
	}

	ResourceComponent* teamResourceComponent = ArgusEntity::GetTeamEntity(updateDisplayParams.m_team).GetComponent<ResourceComponent>();
	ARGUS_RETURN_ON_NULL(teamResourceComponent, ArgusUILog);

	uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		ARGUS_RETURN_ON_NULL(m_resourceWidgetInstances[i], ArgusUILog);
		m_resourceWidgetInstances[i]->UpdateDisplay(static_cast<EResourceType>(i), teamResourceComponent->m_currentResources.m_resourceQuantities[i]);
	}
}