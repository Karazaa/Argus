// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/HealthComponent.h"
#include "ArgusComponentRegistry.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#endif //!UE_BUILD_SHIPPING

// Component shared functionality
uint16 HealthComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_HealthComponents[0];
}

void HealthComponent::DrawComponentDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!ImGui::CollapsingHeader("HealthComponent"))
	{
		return;
	}   
#endif //!UE_BUILD_SHIPPING
}

// Per observable logic
