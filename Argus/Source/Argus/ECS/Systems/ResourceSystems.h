// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDefinitions/ResourceComponent.h"
#include "SystemArgumentDefinitions/ResourceSystemsArgs.h"

class ArgusEntity;
class UResourceSetRecord;
struct FResourceSet;
struct ResourceComponent;

class ResourceSystems
{
public:
	static void RunSystems(float deltaTime);

	static void ProcessResourceExtractionState(const ResourceSystemsArgs& components);
	static void ProcessResourceExtractionTiming(const ResourceSystemsArgs& components);
	static void ProcessResourceDepositing(const ResourceSystemsArgs& components);
	static bool ExtractResources(const ResourceSystemsArgs& components);
	static void DepositResources(const ResourceSystemsArgs& components);
	static void MoveToNearestDepositSink(const ResourceSystemsArgs& components);
	static void MoveToLastExtractionSource(const ResourceSystemsArgs& components);


	static bool CanEntityExtractResourcesFromOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool CanEntityDepositResourcesToOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool CanEntityAffordTeamResourceChange(const ArgusEntity& entity, const FResourceSet& resourceChange);
	static bool ApplyTeamResourceChangeIfAffordable(const ArgusEntity& entity, const FResourceSet& resourceChange);
	
	static ResourceComponent* GetTeamResourceComponentForEntity(const ArgusEntity& entity);

private:
	static void TransferResourcesBetweenComponents(ResourceComponent* sourceComponent, ResourceComponent* targetComponent, const FResourceSet& amount, const UResourceSetRecord* resourceCapacityRecord = nullptr);
};