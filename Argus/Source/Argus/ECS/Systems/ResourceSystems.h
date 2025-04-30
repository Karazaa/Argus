// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDefinitions/ResourceComponent.h"

class ArgusEntity;
struct FResourceSet;
struct ResourceComponent;

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

	static void ProcessResourceExtractionState(const ResourceComponents& components);
	static void ProcessResourceExtractionTiming(const ResourceComponents& components);
	static void ProcessResourceDepositing(const ResourceComponents& components);
	static bool ExtractResources(const ResourceComponents& components);
	static void DepositResources(const ResourceComponents& components);


	static bool CanEntityExtractResourcesFromOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool CanEntityDepositResourcesToOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool CanEntityAffordTeamResourceChange(const ArgusEntity& entity, const FResourceSet& resourceChange);
	static bool ApplyTeamResourceChangeIfAffordable(const ArgusEntity& entity, const FResourceSet& resourceChange);
	
	static ResourceComponent* GetTeamResourceComponentForEntity(const ArgusEntity& entity);

private:
	static bool TransferResourcesBetweenComponents(ResourceComponent* sourceComponent, ResourceComponent* targetComponent, const FResourceSet& amount);
};