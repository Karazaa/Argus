// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DynamicAllocComponentDefinitions/EffortCoefficientSettingsComponent.h"
#include "ArgusEntity.h"

const EffortCoefficientSettingsComponent* EffortCoefficientSettingsComponent::Get()
{
	const EffortCoefficientSettingsComponent* settingsComponent = ArgusEntity::GetSingletonEntity().GetComponent<EffortCoefficientSettingsComponent>();
	ARGUS_RETURN_ON_NULL_POINTER(settingsComponent, ArgusECSLog);
	return settingsComponent;
}