// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/ConstructionComponent.h"
#include "ArgusComponentRegistry.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#include "UObject/ReflectedTypeAccessors.h"
#endif //!UE_BUILD_SHIPPING

// Component shared functionality
uint16 ConstructionComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_ConstructionComponents[0];
}

void ConstructionComponent::DrawComponentDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!ImGui::CollapsingHeader("ConstructionComponent"))
	{
		return;
	}

	if (ImGui::BeginTable("ComponentValues", 2, ImGuiTableFlags_NoSavedSettings))
	{
		ImGui::TableNextColumn();
		ImGui::Text("m_requiredWorkSeconds");
		ImGui::TableNextColumn();
		ImGui::Text("%f", m_requiredWorkSeconds);
		ImGui::TableNextColumn();
		ImGui::Text("m_currentWorkSeconds");
		ImGui::TableNextColumn();
		ImGui::Text("%f", m_currentWorkSeconds);
		ImGui::TableNextColumn();
		ImGui::Text("m_constructionAbilityRecordId");
		ImGui::TableNextColumn();
		ImGui::Text("%d", m_constructionAbilityRecordId);
		ImGui::TableNextColumn();
		ImGui::Text("m_constructionType");
		ImGui::TableNextColumn();
		const char* valueName_m_constructionType = ARGUS_FSTRING_TO_CHAR(StaticEnum<EConstructionType>()->GetNameStringByValue(static_cast<uint8>(m_constructionType)))
		ImGui::Text(valueName_m_constructionType);
		ImGui::TableNextColumn();
		ImGui::Text("m_automaticConstructionTimerHandle");
		ImGui::TableNextColumn();
		ImGui::EndTable();
	}
#endif //!UE_BUILD_SHIPPING
}

// Per observable logic
