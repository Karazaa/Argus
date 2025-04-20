// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDefinitions/ResourceComponent.h"

class ArgusEntity;

class ResourceSystems
{
public:
	static void RunSystems(float deltaTime);

	struct ResourceComponents
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		ResourceComponent* m_resourceComponent = nullptr;
		ResourceExtractionComponent* m_resourceExtractionComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};

	static void ProcessResourceExtraction(const ResourceComponents& components);

	static bool CanEntityAffordResourceChange(const ArgusEntity& entity, const FResourceSet& resourceChange);
	static bool ApplyResourceChangeIfAffordable(const ArgusEntity& entity, const FResourceSet& resourceChange);
	
	static ResourceComponent* GetTeamResourceComponentForEntity(const ArgusEntity& entity);
};