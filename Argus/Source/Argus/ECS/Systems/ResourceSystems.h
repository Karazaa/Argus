// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDefinitions/ResourceComponent.h"

class ArgusEntity;

class ResourceSystems
{
public:
	static bool CanEntityAffordResourceChange(const ArgusEntity& entity, const FResourceSet& resourceChange);
	static bool ApplyResourceChangeIfAffordable(const ArgusEntity& entity, const FResourceSet& resourceChange);
	
	static ResourceComponent* GetTeamResourceComponentForEntity(const ArgusEntity& entity);
};