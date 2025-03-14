// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/HealthComponent.h"
#include "ArgusComponentRegistry.h"

// Component shared functionality
uint16 HealthComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_HealthComponents[0];
}

// Per observable logic
