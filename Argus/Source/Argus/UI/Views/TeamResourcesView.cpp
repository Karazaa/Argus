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

	ArgusEntity teamEntity = ArgusEntity::GetPlayerTeamEntity();
	ObserversComponent* playerTeamObserversComponent = teamEntity.GetComponent<ObserversComponent>();
	ARGUS_RETURN_ON_NULL(playerTeamObserversComponent, ArgusUILog);
	ResourceComponent* playerTeamResourceComponent = teamEntity.GetComponent<ResourceComponent>();
	ARGUS_RETURN_ON_NULL(playerTeamResourceComponent, ArgusUILog);

	UpdateResources(playerTeamResourceComponent->m_currentResources);
	playerTeamObserversComponent->m_ResourceComponentObservers.AddObserver(this);
}

void UTeamResourcesView::NativeDestruct()
{
	ObserversComponent* playerTeamObserversComponent = ArgusEntity::GetPlayerTeamEntity().GetComponent<ObserversComponent>();
	ARGUS_RETURN_ON_NULL(playerTeamObserversComponent, ArgusUILog);

	playerTeamObserversComponent->m_ResourceComponentObservers.RemoveObserver(this);
}

void UTeamResourcesView::OnChanged_m_currentResources(const FResourceSet& oldValue, const FResourceSet& newValue)
{
	UpdateResources(newValue);
}

void UTeamResourcesView::UpdateResources(const FResourceSet& newValue)
{
	uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		ARGUS_RETURN_ON_NULL(m_resourceWidgetInstances[i], ArgusUILog);
		m_resourceWidgetInstances[i]->UpdateDisplay(static_cast<EResourceType>(i), newValue.m_resourceQuantities[i]);
	}
}