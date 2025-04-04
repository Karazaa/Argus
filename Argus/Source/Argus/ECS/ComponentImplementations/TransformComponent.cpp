// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/TransformComponent.h"
#include "ArgusEntity.h"

#if !UE_BUILD_SHIPPING
#include "ArgusStaticData.h"
#include "imgui.h"
#include "UObject/ReflectedTypeAccessors.h"
#endif //!UE_BUILD_SHIPPING

// Component shared functionality
uint16 TransformComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_TransformComponents[0];
}

void TransformComponent::DrawComponentDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!ImGui::CollapsingHeader("TransformComponent"))
	{
		return;
	}

	if (ImGui::BeginTable("ComponentValues", 2, ImGuiTableFlags_NoSavedSettings))
	{
		ImGui::TableNextColumn();
		ImGui::Text("m_location");
		ImGui::TableNextColumn();
		ImGui::Text("(%.2f, %.2f, %.2f)", m_location.X, m_location.Y, m_location.Z);
		ImGui::TableNextColumn();
		ImGui::Text("m_currentVelocity");
		ImGui::TableNextColumn();
		ImGui::Text("(%.2f, %.2f, %.2f)", m_currentVelocity.X, m_currentVelocity.Y, m_currentVelocity.Z);
		ImGui::TableNextColumn();
		ImGui::Text("m_proposedAvoidanceVelocity");
		ImGui::TableNextColumn();
		ImGui::Text("(%.2f, %.2f, %.2f)", m_proposedAvoidanceVelocity.X, m_proposedAvoidanceVelocity.Y, m_proposedAvoidanceVelocity.Z);
		ImGui::TableNextColumn();
		ImGui::Text("m_smoothedYaw");
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", m_smoothedYaw.GetValue());
		ImGui::TableNextColumn();
		ImGui::Text("m_targetYaw");
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", m_targetYaw);
		ImGui::TableNextColumn();
		ImGui::Text("m_desiredSpeedUnitsPerSecond");
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", m_desiredSpeedUnitsPerSecond);
		ImGui::TableNextColumn();
		ImGui::Text("m_radius");
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", m_radius);
		ImGui::TableNextColumn();
		ImGui::Text("m_height");
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", m_height);
		ImGui::EndTable();
	}
#endif //!UE_BUILD_SHIPPING
}

// Per observable logic
