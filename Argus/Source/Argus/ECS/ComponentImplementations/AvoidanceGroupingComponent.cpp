// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/AvoidanceGroupingComponent.h"
#include "ArgusEntity.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#include "UObject/ReflectedTypeAccessors.h"
#endif //!UE_BUILD_SHIPPING

// Component shared functionality
uint16 AvoidanceGroupingComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_AvoidanceGroupingComponents[0];
}

void AvoidanceGroupingComponent::DrawComponentDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!ImGui::CollapsingHeader("AvoidanceGroupingComponent"))
	{
		return;
	}

	if (ImGui::BeginTable("ComponentValues", 2, ImGuiTableFlags_NoSavedSettings))
	{
		ImGui::TableNextColumn();
		ImGui::Text("m_adjacentEntities");
		ImGui::TableNextColumn();
		if (m_adjacentEntities.Num() == 0)
		{
			ImGui::Text("Array is empty");
		}
		else
		{
			ImGui::Text("Size of array = %d", m_adjacentEntities.Num());
		}
		ImGui::TableNextColumn();
		ImGui::Text("m_groupAverageLocation");
		ImGui::TableNextColumn();
		ImGui::Text("(%.2f, %.2f, %.2f)", m_groupAverageLocation.X, m_groupAverageLocation.Y, m_groupAverageLocation.Z);
		ImGui::TableNextColumn();
		ImGui::Text("m_groupId");
		ImGui::TableNextColumn();
		ImGui::Text("%d", m_groupId);
		ImGui::TableNextColumn();
		ImGui::Text("m_numberOfIdleEntities");
		ImGui::TableNextColumn();
		ImGui::Text("%d", m_numberOfIdleEntities);
		ImGui::EndTable();
	}
#endif //!UE_BUILD_SHIPPING
}

// Per observable logic
