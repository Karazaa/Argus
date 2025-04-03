// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/TargetingComponent.h"
#include "ArgusComponentRegistry.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#endif //!UE_BUILD_SHIPPING

// Component shared functionality
uint16 TargetingComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_TargetingComponents[0];
}

void TargetingComponent::DrawComponentDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!ImGui::CollapsingHeader("TargetingComponent"))
	{
		return;
	}

	if (ImGui::BeginTable("ComponentValues", 2, ImGuiTableFlags_NoSavedSettings))
	{
		ImGui::TableNextColumn();
		ImGui::Text("m_targetLocation");
		ImGui::TableNextColumn();
		if (m_targetLocation.IsSet())
		{
			ImGui::Text("{%f, %f, %f}", m_targetLocation.GetValue().X, m_targetLocation.GetValue().Y, m_targetLocation.GetValue().Z);
		}
		ImGui::TableNextColumn();
		ImGui::Text("m_meleeRange");
		ImGui::TableNextColumn();
		ImGui::Text("%f", m_meleeRange);
		ImGui::TableNextColumn();
		ImGui::Text("m_rangedRange");
		ImGui::TableNextColumn();
		ImGui::Text("%f", m_rangedRange);
		ImGui::TableNextColumn();
		ImGui::Text("m_targetEntityId");
		ImGui::TableNextColumn();
		ImGui::Text("%d", m_targetEntityId);
		ImGui::EndTable();
	}
#endif //!UE_BUILD_SHIPPING
}

// Per observable logic
