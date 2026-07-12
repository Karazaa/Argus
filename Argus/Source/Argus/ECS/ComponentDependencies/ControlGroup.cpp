// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ControlGroup.h"
#include "Serialization/Archive.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#endif // !UE_BUILD_SHIPPING

FArchive& operator<<(FArchive& archive, ControlGroup& controlGroup)
{
	controlGroup.m_entityIds.BulkSerialize(archive);
	return archive;
}

#if !UE_BUILD_SHIPPING
void ControlGroup::DrawImGuiDebug() const
{
	ImGui::Text("Control Group max is currently = %d", m_entityIds.Max());
	if (m_entityIds.Num() == 0)
	{
		ImGui::Text("Control Group is empty");
	}
	else
	{
		ImGui::Text("Size of Control Group = %d", m_entityIds.Num());
		for (int32 i = 0; i < m_entityIds.Num(); ++i)
		{
			ImGui::Text("%d", m_entityIds[i]);
		}
	}
}
#endif // !UE_BUILD_SHIPPING