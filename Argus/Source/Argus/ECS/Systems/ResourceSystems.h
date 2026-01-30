// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDefinitions/IdentityComponent.h"
#include "ComponentDefinitions/ResourceComponent.h"
#include "ComponentDependencies/ResourceSet.h"
#include "SystemArgumentDefinitions/ResourceSystemsArgs.h"

class ArgusEntity;
class UArgusEntityTemplate;
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
	static void DepositResources(const ResourceSystemsArgs& components, ArgusEntity targetEntity);
	static void MoveToNearestDepositSink(const ResourceSystemsArgs& components);
	static void MoveToLastExtractionSource(const ResourceSystemsArgs& components);


	static bool CanEntityActAsSinkToAnotherEntitySource(ArgusEntity entity, ArgusEntity otherEntity);
	static bool CanEntityTemplateActAsSinkToEntitySource(const UArgusEntityTemplate* entityTemplate, ArgusEntity otherEntity);
	static bool CanEntityExtractResourcesFromOtherEntity(ArgusEntity entity, ArgusEntity otherEntity);
	static bool CanEntityDepositResourcesToOtherEntity(ArgusEntity entity, ArgusEntity otherEntity);
	static bool CanEntityAffordTeamResourceChange(ArgusEntity entity, const FResourceSet& resourceChange);
	static bool ApplyTeamResourceChangeIfAffordable(ArgusEntity entity, const FResourceSet& resourceChange);
	static bool ApplyTeamResourceChangeIfAffordable(ETeam team, const FResourceSet& resourceChange);

	static ResourceComponent* GetTeamResourceComponentForEntity(ArgusEntity entity);
	static ResourceComponent* GetTeamResourceComponentForTeam(ETeam team);

private:
	static void TransferResourcesBetweenComponents(ResourceComponent* sourceComponent, ResourceComponent* targetComponent, const FResourceSet& amount, const UResourceSetRecord* resourceCapacityRecord = nullptr);
	static void ClearResourceGatheringForEntity(const ResourceSystemsArgs& components);
};