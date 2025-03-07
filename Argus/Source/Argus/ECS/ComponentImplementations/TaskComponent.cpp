// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ComponentDefinitions/TaskComponent.h"
#include "ArgusComponentRegistry.h"

uint16 TaskComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_TaskComponents[0];
}

void TaskComponent::Set_m_baseState(BaseState newState)
{
	BaseState oldState = m_baseState;
	m_baseState = newState;

	ObserversComponent* observersComponent = ArgusComponentRegistry::GetComponent<ObserversComponent>(GetOwningEntityId());
	if (!observersComponent)
	{
		// TODO JAMES: error here.
		return;
	}

	observersComponent->m_taskComponentObservers.OnChanged_m_baseState(oldState, newState);
}