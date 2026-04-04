// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DynamicAllocComponentDefinitions/DecalSystemsSettingsComponent.h"
#include "ArgusEntity.h"

const DecalSystemsSettingsComponent* DecalSystemsSettingsComponent::Get()
{
	const DecalSystemsSettingsComponent* settingsComponent = ArgusEntity::GetSingletonEntity().GetComponent<DecalSystemsSettingsComponent>();
	ARGUS_RETURN_ON_NULL_POINTER(settingsComponent, ArgusECSLog);
	return settingsComponent;
}