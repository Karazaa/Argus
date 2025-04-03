// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ComponentDefinitions/TaskComponent.h"
#include "ArgusComponentRegistry.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#include "UObject/ReflectedTypeAccessors.h"
#endif //!UE_BUILD_SHIPPING

// Component shared functionality
uint16 TaskComponent::GetOwningEntityId() const
{
	return this - &ArgusComponentRegistry::s_TaskComponents[0];
}

void TaskComponent::DrawComponentDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!ImGui::CollapsingHeader("TaskComponent"))
	{
		return;
	}

	if (ImGui::BeginTable("ComponentValues", 2, ImGuiTableFlags_NoSavedSettings))
	{
		ImGui::TableNextColumn();
		ImGui::Text("m_spawnedFromArgusActorRecordId");
		ImGui::TableNextColumn();
		ImGui::Text("%d", m_spawnedFromArgusActorRecordId);
		ImGui::TableNextColumn();
		ImGui::Text("m_baseState");
		ImGui::TableNextColumn();
		const char* valueName_m_baseState = ARGUS_FSTRING_TO_CHAR(StaticEnum<EBaseState>()->GetNameStringByValue(static_cast<uint8>(m_baseState)))
		ImGui::Text(valueName_m_baseState);
		ImGui::TableNextColumn();
		ImGui::Text("m_movementState");
		ImGui::TableNextColumn();
		const char* valueName_m_movementState = ARGUS_FSTRING_TO_CHAR(StaticEnum<EMovementState>()->GetNameStringByValue(static_cast<uint8>(m_movementState)))
		ImGui::Text(valueName_m_movementState);
		ImGui::TableNextColumn();
		ImGui::Text("m_spawningState");
		ImGui::TableNextColumn();
		const char* valueName_m_spawningState = ARGUS_FSTRING_TO_CHAR(StaticEnum<ESpawningState>()->GetNameStringByValue(static_cast<uint8>(m_spawningState)))
		ImGui::Text(valueName_m_spawningState);
		ImGui::TableNextColumn();
		ImGui::Text("m_abilityState");
		ImGui::TableNextColumn();
		const char* valueName_m_abilityState = ARGUS_FSTRING_TO_CHAR(StaticEnum<EAbilityState>()->GetNameStringByValue(static_cast<uint8>(m_abilityState)))
		ImGui::Text(valueName_m_abilityState);
		ImGui::TableNextColumn();
		ImGui::Text("m_constructionState");
		ImGui::TableNextColumn();
		const char* valueName_m_constructionState = ARGUS_FSTRING_TO_CHAR(StaticEnum<EConstructionState>()->GetNameStringByValue(static_cast<uint8>(m_constructionState)))
		ImGui::Text(valueName_m_constructionState);
		ImGui::TableNextColumn();
		ImGui::Text("m_combatState");
		ImGui::TableNextColumn();
		const char* valueName_m_combatState = ARGUS_FSTRING_TO_CHAR(StaticEnum<ECombatState>()->GetNameStringByValue(static_cast<uint8>(m_combatState)))
		ImGui::Text(valueName_m_combatState);
		ImGui::EndTable();
	}
#endif //!UE_BUILD_SHIPPING
}

// Per observable logic
void TaskComponent::Set_m_baseState(EBaseState newValue)
{
	EBaseState oldValue = m_baseState;
	m_baseState = newValue;

	ObserversComponent* observersComponent = ArgusComponentRegistry::GetComponent<ObserversComponent>(GetOwningEntityId());
	if (!observersComponent)
	{
		// TODO JAMES: error here.
		return;
	}

	observersComponent->m_TaskComponentObservers.OnChanged_m_baseState(oldValue, newValue);
}
