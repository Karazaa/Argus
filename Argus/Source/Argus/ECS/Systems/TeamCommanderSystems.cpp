// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "ArgusStaticData.h"
#include "DataComponentDefinitions/ResourceComponentData.h"
#include "Systems/ResourceSystems.h"

void TeamCommanderSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TeamCommanderSystems::RunSystems);

	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);
	ArgusEntity::IterateSystemsArgs<TeamCommanderSystemsArgs>(TeamCommanderSystems::UpdateTeamCommanderPerEntity);
	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::UpdateTeamCommanderPriorities);
	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::ActUponUpdatesPerCommanderEntity);
}

void TeamCommanderSystems::InitializeRevealedAreas(TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float areasPerWidth = ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength);
	const int32 numAreas = FMath::FloorToInt32(FMath::Square(areasPerWidth));
	teamCommanderComponent->m_revealedAreas.SetNum(numAreas, false);
}

void TeamCommanderSystems::ClearUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	teamCommanderComponent->m_idleEntityIdsForTeam.Reset();
	teamCommanderComponent->m_seenResourceSourceEntityIds.Reset();
	teamCommanderComponent->m_resourceSinkEntityIds.Reset();
	teamCommanderComponent->m_priorities.Reset();
}

void TeamCommanderSystems::UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_identityComponent->m_team == ETeam::None)
	{
		ArgusEntity::IterateTeamEntities([&components](ArgusEntity teamCommanderEntity)
		{
			TeamCommanderSystems::UpdateTeamCommanderPerNeutralEntity(components, teamCommanderEntity);
		});
		return;
	}

	TeamCommanderComponent* teamCommanderComponent = ArgusEntity::GetTeamEntity(components.m_identityComponent->m_team).GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	UpdateTeamCommanderPerEntityOnTeam(components, teamCommanderComponent);
}

void TeamCommanderSystems::UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntityOnTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_entity.IsIdle() && !components.m_entity.IsPassenger())
	{
		teamCommanderComponent->m_idleEntityIdsForTeam.Add(components.m_entity.GetId());
	}

	if (components.m_resourceComponent && components.m_resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Sink)
	{
		teamCommanderComponent->m_resourceSinkEntityIds.Add(components.m_entity.GetId());
	}

	UpdateRevealedAreasPerEntityOnTeam(components, teamCommanderComponent);
}

void TeamCommanderSystems::UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntityOnOtherTeam);

	TeamCommanderComponent* teamCommanderComponent = teamCommanderEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (ResourceComponent* resourceComponent = components.m_entity.GetComponent<ResourceComponent>())
	{
		if (resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Source && components.m_identityComponent->WasEverSeenBy(teamCommanderComponent->m_teamToCommand))
		{
			teamCommanderComponent->m_seenResourceSourceEntityIds.Add(components.m_entity.GetId());
		}
	}
}

void TeamCommanderSystems::UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateRevealedAreasPerEntityOnTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return;
	}

	const int32 areaIndex = GetAreaIndexFromWorldSpaceLocation(components, teamCommanderComponent);
	if (areaIndex >= 0)
	{
		teamCommanderComponent->m_revealedAreas[areaIndex] = true;
	}
}

void TeamCommanderSystems::UpdateTeamCommanderPriorities(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPriorities);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (uint8 i = 0u; i < static_cast<uint8>(ETeamCommanderDirective::Count); ++i)
	{
		const ETeamCommanderDirective directiveToEvaluate = static_cast<ETeamCommanderDirective>(i);
		TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
		priority.m_directive = directiveToEvaluate;
		switch (priority.m_directive)
		{
			case ETeamCommanderDirective::ConstructResourceSink:
				UpdateConstructResourceSinkTeamPriority(teamCommanderComponent, priority);
				continue;
			case ETeamCommanderDirective::ExtractResources:
				UpdateResourceExtractionTeamPriority(teamCommanderComponent, priority);
				continue;
			case ETeamCommanderDirective::Scout:
				UpdateScoutingTeamPriority(teamCommanderComponent, priority);
				continue;
			default:
				continue;
		}
	}

	teamCommanderComponent->m_priorities.Sort();
}

void TeamCommanderSystems::UpdateConstructResourceSinkTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateConstructResourceSinkTeamPriority);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ConstructResourceSink)
	{
		return;
	}

	if (teamCommanderComponent->m_resourceSinkEntityIds.Num() == 0 && teamCommanderComponent->m_seenResourceSourceEntityIds.Num() > 0)
	{
		priority.m_weight = 2.0f;
		return;
	}

	// TODO JAMES: (Design question) What is the right proportion of resource sinks to resource sources? Do we need to define acceptable ranges from source to sink?
	if (teamCommanderComponent->m_resourceSinkEntityIds.Num() < teamCommanderComponent->m_seenResourceSourceEntityIds.Num())
	{
		priority.m_weight = 1.0f;
		return;
	}

	priority.m_weight = 0.0f;
}

void TeamCommanderSystems::UpdateResourceExtractionTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateResourceExtractionTeamPriority);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ExtractResources)
	{
		return;
	}

	priority.m_weight = (teamCommanderComponent->m_seenResourceSourceEntityIds.Num() > 0 && teamCommanderComponent->m_resourceSinkEntityIds.Num() > 0) ? 1.0f : 0.0f;
}

void TeamCommanderSystems::UpdateScoutingTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateScoutingTeamPriority);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::Scout)
	{
		return;
	}

	// TODO JAMES: We should probably massively de-prioritize this based on how much of the map has been discovered. 

	priority.m_weight = 1.0f;
}

void TeamCommanderSystems::ActUponUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ActUponUpdatesPerCommanderEntity);

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	// TODO JAMES: Are there any circumstances in which we want to allow the AI to control a player team?
	if (teamCommanderComponent->m_teamToCommand == inputInterfaceComponent->m_activePlayerTeam)
	{
		return;
	}

	for (int32 i = 0; i < teamCommanderComponent->m_idleEntityIdsForTeam.Num(); ++i)
	{
		AssignIdleEntityToWork(ArgusEntity::RetrieveEntity(teamCommanderComponent->m_idleEntityIdsForTeam[i]), teamCommanderComponent);
	}
}

void TeamCommanderSystems::AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignIdleEntityToWork);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (int32 i = 0; i < teamCommanderComponent->m_priorities.Num(); ++i)
	{
		if (AssignIdleEntityToDirectiveIfAble(idleEntity, teamCommanderComponent, teamCommanderComponent->m_priorities[i]))
		{
			return;
		}
	}
}

bool TeamCommanderSystems::AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignIdleEntityToDirectiveIfAble);
	switch (priority.m_directive)
	{
		case ETeamCommanderDirective::ConstructResourceSink:
			return AssignEntityToConstructResourceSinkIfAble(idleEntity, teamCommanderComponent, priority);
		case ETeamCommanderDirective::ExtractResources:
			return AssignEntityToResourceExtractionIfAble(idleEntity, teamCommanderComponent);
		case ETeamCommanderDirective::Scout:
			return AssignEntityToScoutingIfAble(idleEntity, teamCommanderComponent);
		default:
			break;
	}

	return false;
}

bool TeamCommanderSystems::AssignEntityToConstructResourceSinkIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToResourceExtractionIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	EAbilityIndex abilityIndex = EAbilityIndex::None;
	const UAbilityRecord* constructionAbility = GetConstructResourceSinkAbility(entity, abilityIndex);
	if (!constructionAbility || abilityIndex == EAbilityIndex::None)
	{
		return false;
	}

	// TODO JAMES: For now, temporarily nuking construction priority so that only one entity is assigned to construction at a time. This will stay in place until the actual construction logic is authored.
	FindTargetLocForConstructResourceSink(entity, constructionAbility, teamCommanderComponent);
	if (TaskComponent* taskComponent = entity.GetComponent<TaskComponent>())
	{
		taskComponent->m_directiveFromTeamCommander = priority.m_directive;
	}
	priority.m_weight = 0.0f;
	teamCommanderComponent->m_priorities.Sort();

	return true;
}

bool TeamCommanderSystems::AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToResourceExtractionIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!taskComponent || !targetingComponent)
	{
		return false;
	}

	ArgusEntity closestEntity = ArgusEntity::k_emptyEntity;
	float closestDistanceSquared = FLT_MAX;
	for (int32 i = 0; i < teamCommanderComponent->m_seenResourceSourceEntityIds.Num(); ++i)
	{
		ArgusEntity resourceSourceEntity = ArgusEntity::RetrieveEntity(teamCommanderComponent->m_seenResourceSourceEntityIds[i]);
		if (!ResourceSystems::CanEntityExtractResourcesFromOtherEntity(entity, resourceSourceEntity))
		{
			continue;
		}

		const float distanceSquared = entity.GetDistanceSquaredToOtherEntity(resourceSourceEntity);
		if (distanceSquared < closestDistanceSquared)
		{
			closestDistanceSquared = distanceSquared;
			closestEntity = resourceSourceEntity;
		}
	}

	if (!closestEntity)
	{
		return false;
	}

	targetingComponent->SetEntityTarget(closestEntity.GetId());
	taskComponent->m_resourceExtractionState = EResourceExtractionState::DispatchedToExtract;
	taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	taskComponent->m_directiveFromTeamCommander = ETeamCommanderDirective::ExtractResources;
	return true;
}

bool TeamCommanderSystems::AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToScoutingIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	TeamCommanderSystemsArgs components;
	if (!components.PopulateArguments(entity) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return  false;
	}

	const int32 unrevealedIndex = GetClosestUnrevealedAreaToEntity(components, teamCommanderComponent);
	if (unrevealedIndex < 0)
	{
		return false;
	}

	components.m_targetingComponent->SetLocationTarget(GetWorldSpaceLocationFromAreaIndex(unrevealedIndex, teamCommanderComponent));
	components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
	components.m_taskComponent->m_directiveFromTeamCommander = ETeamCommanderDirective::Scout;
	return true;
}

int32 TeamCommanderSystems::GetAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, -1);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return -1;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, -1);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float areasPerDimension = ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength);

	float xValue = ArgusMath::SafeDivide(components.m_transformComponent->m_location.Y + spatialPartitioningComponent->m_validSpaceExtent, teamCommanderComponent->m_revealedAreaDimensionLength);
	float yValue = ArgusMath::SafeDivide((-components.m_transformComponent->m_location.X) + spatialPartitioningComponent->m_validSpaceExtent, teamCommanderComponent->m_revealedAreaDimensionLength);

	int32 xValue32 = FMath::FloorToInt32(xValue);
	int32 yValue32 = FMath::FloorToInt32(yValue);

	return (yValue32 * FMath::FloorToInt32(areasPerDimension)) + xValue32;
}

FVector TeamCommanderSystems::GetWorldSpaceLocationFromAreaIndex(int32 areaIndex, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, FVector::ZeroVector);
	if (areaIndex < 0)
	{
		return FVector::ZeroVector;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, FVector::ZeroVector);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const int32 areasPerDimension = FMath::FloorToInt32(ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength));

	int32 xCoordinate, yCoordinate;
	ConvertAreaIndexToAreaCoordinates(areaIndex, areasPerDimension, xCoordinate, yCoordinate);

	const float xOffset = ((static_cast<float>(xCoordinate) + 0.5f) * teamCommanderComponent->m_revealedAreaDimensionLength);
	const float yOffset = ((static_cast<float>(yCoordinate) + 0.5f) * teamCommanderComponent->m_revealedAreaDimensionLength);

	FVector output = FVector::ZeroVector;
	output.Y = xOffset - spatialPartitioningComponent->m_validSpaceExtent;
	output.X = spatialPartitioningComponent->m_validSpaceExtent - yOffset;
	return output;
}

int32 TeamCommanderSystems::GetClosestUnrevealedAreaToEntity(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, -1);

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, -1);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const int32 areasPerDimension = FMath::FloorToInt32(ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength));
	int32 entityXCoordinate, entityYCoordinate;
	ConvertAreaIndexToAreaCoordinates(GetAreaIndexFromWorldSpaceLocation(components, teamCommanderComponent), areasPerDimension, entityXCoordinate, entityYCoordinate);

	if (entityXCoordinate < 0 || entityYCoordinate < 0)
	{
		return -1;
	}

	for (int32 i = 1; i < areasPerDimension; ++i)
	{
		const int32 leftBoundX = FMath::Max(entityXCoordinate - i, 0);
		const int32 rightBoundX = FMath::Min(entityXCoordinate + i, (areasPerDimension - 1));
		const int32 upperBoundY = FMath::Max(entityYCoordinate - i, 0);
		const int32 lowerBoundY = FMath::Min(entityYCoordinate + i, (areasPerDimension - 1));

		TArray<int32> validAreasNearby;
		for (int32 j = -i; j <= i; ++j)
		{
			const int32 xBoundY = FMath::Min(FMath::Max(entityYCoordinate + i, 0), (areasPerDimension - 1));
			const int32 yBoundX = FMath::Min(FMath::Max(entityXCoordinate + i, 0), (areasPerDimension - 1));

			int32 indexToCheck;
			ConvertAreaCoordinatesToAreaIndex(leftBoundX, xBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck > 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(rightBoundX, xBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck > 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(yBoundX, upperBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck > 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(yBoundX, lowerBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck > 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
		}

		if (validAreasNearby.Num() > 0)
		{
			return validAreasNearby[FMath::RandRange(0, validAreasNearby.Num() - 1)];
		}
	}

	return -1;
}

void TeamCommanderSystems::ConvertAreaIndexToAreaCoordinates(int32 areaIndex, int32 areasPerDimension, int32& xCoordinate, int32& yCoordinate)
{
	if (areaIndex < 0)
	{
		xCoordinate = -1;
		yCoordinate = -1;
	}

	yCoordinate = ArgusMath::SafeDivide(areaIndex, areasPerDimension);
	xCoordinate = areaIndex % areasPerDimension;
}

void TeamCommanderSystems::ConvertAreaCoordinatesToAreaIndex(int32 xCoordinate, int32 yCoordinate, int32 areasPerDimension, int32& areaIndex)
{
	areaIndex = (yCoordinate * areasPerDimension) + xCoordinate;
}

const UAbilityRecord* TeamCommanderSystems::GetConstructResourceSinkAbility(ArgusEntity entity, EAbilityIndex& abilityIndex)
{
	ARGUS_TRACE(TeamCommanderSystems::GetConstructResourceSinkAbility);

	abilityIndex = EAbilityIndex::None;
	AbilityComponent* abilityComponent = entity.GetComponent<AbilityComponent>();
	if (!abilityComponent)
	{
		return nullptr;
	}

	const UAbilityRecord* constructionRecord = nullptr;
	abilityComponent->IterateActiveAbilityIds([&constructionRecord, &abilityIndex](uint32 abilityRecordId, EAbilityIndex iteratedAbilityIndex)
	{
		const UAbilityRecord* record = ArgusStaticData::GetRecord<UAbilityRecord>(abilityRecordId);
		if (!record)
		{
			return;
		}

		if (DoesAbilityConstructResourceSink(record))
		{
			abilityIndex = iteratedAbilityIndex;
			constructionRecord = record;
		}
	});

	return constructionRecord;
}

bool TeamCommanderSystems::DoesAbilityConstructResourceSink(const UAbilityRecord* abilityRecord)
{
	if (!abilityRecord)
	{
		return false;
	}

	for (int32 i = 0; i < abilityRecord->m_abilityEffects.Num(); ++i)
	{
		if (abilityRecord->m_abilityEffects[i].m_abilityType != EAbilityTypes::Construct)
		{
			continue;
		}

		const UArgusActorRecord* actorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(abilityRecord->m_abilityEffects[i].m_argusActorRecordId);
		if (!actorRecord)
		{
			continue;
		}

		const UArgusEntityTemplate* entityTemplate = actorRecord->m_entityTemplate.LoadAndStorePtr();
		if (!entityTemplate)
		{
			continue;
		}

		for (int32 j = 0; j < entityTemplate->m_componentData.Num(); ++j)
		{
			// TODO JAMES: Component data might not be loaded right now. How can we support async loading here? Should component data have a hard reference so it can never be garbage collected?

			ARGUS_TRACE(TeamCommanderSystems::LoadComponentData);
			const UResourceComponentData* resourceComponentData = Cast<UResourceComponentData>(entityTemplate->m_componentData[j].LoadSynchronous());
			if (!resourceComponentData)
			{
				continue;
			}

			if (resourceComponentData->m_resourceComponentOwnerType == EResourceComponentOwnerType::Sink)
			{
				return true;
			}
		}
	}

	return false;
}

void TeamCommanderSystems::FindTargetLocForConstructResourceSink(ArgusEntity entity, const UAbilityRecord* abilityRecord, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(abilityRecord, ArgusECSLog);
}
