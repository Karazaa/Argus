// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderPriorities.h"
#include "ArgusMacros.h"
#include "Serialization/Archive.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#endif // !UE_BUILD_SHIPPING

FArchive& operator<<(FArchive& archive, ResourceSourceExtractionData& resourceSourceExtractionData)
{
	archive << resourceSourceExtractionData.m_resourceSourceEntityId;
	archive << resourceSourceExtractionData.m_resourceSinkEntityId;
	archive << resourceSourceExtractionData.m_resourceExtractorEntityId;
	return archive;
}

#if !UE_BUILD_SHIPPING
void TeamCommanderPriority::DrawImGuiDebug() const
{
	ImGui::BeginTable("TeamCommanderPriority", 4, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp);
	ImGui::TableNextColumn();
	ImGui::Text("%.2f", m_weight);
	ImGui::TableNextColumn();
	const char* directive = ARGUS_FSTRING_TO_CHAR(StaticEnum<ETeamCommanderDirective>()->GetNameStringByValue(static_cast<uint8>(m_directive)));
	ImGui::Text(directive);
	ImGui::TableNextColumn();
	if (m_entityCategory.m_entityCategoryType != EEntityCategoryType::Count)
	{
		const char* entityCategory = ARGUS_FSTRING_TO_CHAR(StaticEnum<EEntityCategoryType>()->GetNameStringByValue(static_cast<uint8>(m_entityCategory.m_entityCategoryType)));
		ImGui::Text(entityCategory);
	}

	ImGui::TableNextColumn();
	if (m_entityCategory.m_resourceType != EResourceType::Count)
	{
		const char* resourceType = ARGUS_FSTRING_TO_CHAR(StaticEnum<EResourceType>()->GetNameStringByValue(static_cast<uint8>(m_entityCategory.m_resourceType)));
		ImGui::Text(resourceType);
	}
	else if (m_entityCategory.m_entityCategoryType == EEntityCategoryType::Combatant)
	{
		const char* attackCapability = ARGUS_FSTRING_TO_CHAR(StaticEnum<ERangedAttackCapability>()->GetNameStringByValue(static_cast<uint8>(m_entityCategory.m_attackCapability)));
		ImGui::Text(attackCapability);
	}
	ImGui::EndTable();
}

void ResourceSourceExtractionData::DrawImGuiDebug() const
{
	ImGui::BeginTable("ResourceSourceExtractionData", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp);
	ImGui::TableNextColumn();
	ImGui::Text("Source Entity Id");
	ImGui::TableNextColumn();
	ImGui::Text("%d", m_resourceSourceEntityId);
	ImGui::TableNextColumn();
	ImGui::Text("Sink Entity Id");
	ImGui::TableNextColumn();
	ImGui::Text("%d", m_resourceSinkEntityId);
	ImGui::TableNextColumn();
	ImGui::Text("Extractor Entity Id");
	ImGui::TableNextColumn();
	ImGui::Text("%d", m_resourceExtractorEntityId);
	ImGui::EndTable();
}
#endif // !UE_BUILD_SHIPPING