// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/TimerComponent.h"
#include "ArgusComponentRegistry.h"

// Component shared functionality
uint16 TimerComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_TimerComponents[0];
}

// Per observable logic
