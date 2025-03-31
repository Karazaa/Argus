// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/CombatComponent.h"
#include "ArgusComponentRegistry.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#endif //!UE_BUILD_SHIPPING

// Component shared functionality
uint16 CombatComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_CombatComponents[0];
}

void CombatComponent::DrawComponentDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!ImGui::CollapsingHeader("CombatComponent"))
	{
		return;
	}   
#endif //!UE_BUILD_SHIPPING
}

// Per observable logic
