// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DynamicAllocComponentDefinitions/GlobalSettingsComponent.h"
#include "ArgusEntity.h"

const GlobalSettingsComponent* GlobalSettingsComponent::Get()
{
	const GlobalSettingsComponent* settingsComponent = ArgusEntity::GetSingletonEntity().GetComponent<GlobalSettingsComponent>();
	ARGUS_RETURN_ON_NULL_POINTER(settingsComponent, ArgusECSLog);
	return settingsComponent;
}