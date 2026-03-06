// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems_AssignEntities.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/AbilitySystems.h"
#include "Systems/ResourceSystems.h"
#include "Systems/SpatialPartitioningSystems.h"
#include "Systems/TeamCommanderSystems.h"
#include "Systems/TransformSystems.h"

void TeamCommanderSystems_AssignEntities::RunSystems()
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::RunSystems);

	ArgusEntity::IterateTeamEntities(TeamCommanderSystems_AssignEntities::ActUponUpdatesPerCommanderEntity);
}

void TeamCommanderSystems_AssignEntities::ActUponUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::ActUponUpdatesPerCommanderEntity);

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

void TeamCommanderSystems_AssignEntities::AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::AssignIdleEntityToWork);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (int32 i = 0; i < teamCommanderComponent->m_priorities.Num(); ++i)
	{
		if (AssignIdleEntityToDirectiveIfAble(idleEntity, teamCommanderComponent, teamCommanderComponent->m_priorities[i]))
		{
			return;
		}
	}
}

bool TeamCommanderSystems_AssignEntities::AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::AssignIdleEntityToDirectiveIfAble);

	if (priority.m_weight < 0.0f)
	{
		return false;
	}

	switch (priority.m_directive)
	{
		case ETeamCommanderDirective::ConstructResourceSink:
			return AssignEntityToConstructResourceSinkIfAble(idleEntity, teamCommanderComponent, priority);
		case ETeamCommanderDirective::ExtractResources:
			return AssignEntityToResourceExtractionIfAble(idleEntity, teamCommanderComponent);
		case ETeamCommanderDirective::SpawnUnit:
			return AssignEntityToSpawnUnitIfAble(idleEntity, teamCommanderComponent, priority);
		case ETeamCommanderDirective::Scout:
			return AssignEntityToScoutingIfAble(idleEntity, teamCommanderComponent);
		default:
			break;
	}

	return false;
}

bool TeamCommanderSystems_AssignEntities::AssignEntityToConstructResourceSinkIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::AssignEntityToConstructResourceSinkIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_entityCategory.m_resourceType == EResourceType::Count)
	{
		return false;
	}

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	TArray<TPair<const UAbilityRecord*, EAbilityIndex>> abilityIndexPairs;
	if (!AbilitySystems::GetSpawnEntityCategoryAbilities(entity, priority.m_entityCategory, abilityIndexPairs))
	{
		return false;
	}

	if (!FindTargetLocForConstructResourceSink(entity, abilityIndexPairs, teamCommanderComponent, priority.m_entityCategory.m_resourceType))
	{
		return false;
	}

	taskComponent->m_abilityState = AbilitySystems::GetProcessAbilityStateForAbilityIndex(abilityIndexPairs[0].Value);
	taskComponent->m_directiveFromTeamCommander = priority.m_directive;
	priority.m_weight = 0.0f;
	teamCommanderComponent->m_priorities.Sort();

	return true;
}

bool TeamCommanderSystems_AssignEntities::AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::AssignEntityToResourceExtractionIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!taskComponent || !targetingComponent)
	{
		return false;
	}

	ArgusEntity closestEntity = ArgusEntity::k_emptyEntity;
	float closestDistanceSquared = FLT_MAX;
	ResourceSourceExtractionData* pointerToClosestExtractionData = nullptr;
	teamCommanderComponent->IterateAllSeenResourceSources([entity, &closestEntity, &closestDistanceSquared, &pointerToClosestExtractionData](ResourceSourceExtractionData& data)
	{
		const bool isCurrentEntity = data.m_resourceExtractorEntityId == entity.GetId();
		if (data.m_resourceSourceEntityId == ArgusECSConstants::k_maxEntities || (data.m_resourceExtractorEntityId != ArgusECSConstants::k_maxEntities && !isCurrentEntity))
		{
			return false;
		}

		ArgusEntity resourceSourceEntity = ArgusEntity::RetrieveEntity(data.m_resourceSourceEntityId);
		if (!ResourceSystems::CanEntityExtractResourcesFromOtherEntity(entity, resourceSourceEntity))
		{
			return false;
		}

		if (isCurrentEntity)
		{
			closestEntity = resourceSourceEntity;
			pointerToClosestExtractionData = &data;
			return true;
		}

		const float distanceSquared = entity.GetDistanceSquaredToOtherEntity(resourceSourceEntity);
		if (distanceSquared < closestDistanceSquared)
		{
			closestDistanceSquared = distanceSquared;
			closestEntity = resourceSourceEntity;
			pointerToClosestExtractionData = &data;
		}

		return false;
	});

	if (!closestEntity || !pointerToClosestExtractionData)
	{
		return false;
	}

	targetingComponent->SetEntityTarget(closestEntity.GetId());
	taskComponent->m_resourceExtractionState = EResourceExtractionState::DispatchedToExtract;
	taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	taskComponent->m_directiveFromTeamCommander = ETeamCommanderDirective::ExtractResources;
	pointerToClosestExtractionData->m_resourceExtractorEntityId = entity.GetId();
	return true;
}

bool TeamCommanderSystems_AssignEntities::AssignEntityToSpawnUnitIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::AssignEntityToSpawnUnitIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	const SpawningComponent* spawningComponent = entity.GetComponent<SpawningComponent>();
	if (!taskComponent || !spawningComponent)
	{
		return false;
	}

	TArray<TPair<const UAbilityRecord*, EAbilityIndex>> abilityIndexPairs;
	if (!AbilitySystems::GetSpawnEntityCategoryAbilities(entity, priority.m_entityCategory, abilityIndexPairs))
	{
		return false;
	}

	taskComponent->m_abilityState = AbilitySystems::GetProcessAbilityStateForAbilityIndex(abilityIndexPairs[0].Value);
	taskComponent->m_directiveFromTeamCommander = priority.m_directive;
	priority.m_weight -= 1.0f;
	teamCommanderComponent->m_priorities.Sort();
	return true;
}

bool TeamCommanderSystems_AssignEntities::AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems_AssignEntities::AssignEntityToScoutingIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	if (!entity.IsMoveable())
	{
		return false;
	}

	TeamCommanderSystemsArgs components;
	if (!components.PopulateArguments(entity) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return  false;
	}

	const int32 unrevealedIndex = TeamCommanderSystems::GetClosestUnrevealedAreaToEntity(components, teamCommanderComponent);
	if (unrevealedIndex < 0)
	{
		return false;
	}

	components.m_targetingComponent->SetLocationTarget(TeamCommanderSystems::GetWorldSpaceLocationFromAreaIndex(unrevealedIndex, teamCommanderComponent));
	components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
	components.m_taskComponent->m_directiveFromTeamCommander = ETeamCommanderDirective::Scout;
	return true;
}

bool TeamCommanderSystems_AssignEntities::FindTargetLocForConstructResourceSink(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderComponent* teamCommanderComponent, EResourceType type)
{
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	WorldReferenceComponent* worldReferenceComponent = ArgusEntity::GetSingletonEntity().GetComponent<WorldReferenceComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(worldReferenceComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL_BOOL(worldReferenceComponent->m_worldPointer, ArgusECSLog);

	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return false;
	}

	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> >& sources = teamCommanderComponent->GetSeenSourceExtractionDataForResourceType(type);

	int32 pairIndex = -1;
	int32 extractionDataIndex = -1;
	ArgusEntity nearestResourceSource = GetNearestSeenResourceSourceToEntity(entity, abilityIndexPairs, teamCommanderComponent, type, pairIndex, extractionDataIndex);
	if (!nearestResourceSource || pairIndex < 0 || extractionDataIndex < 0 || pairIndex >= abilityIndexPairs.Num() || extractionDataIndex >= sources.Num())
	{
		return false;
	}

	TransformComponent* resourceSourceTransformComponent = nearestResourceSource.GetComponent<TransformComponent>();
	TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(resourceSourceTransformComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL_BOOL(transformComponent, ArgusECSLog);

	FVector2D fromSinkToEntity = FVector2D(transformComponent->m_location - resourceSourceTransformComponent->m_location).GetSafeNormal();

	const float safeZoneDistance = AbilitySystems::GetResourceBufferRadiusOfConstructionAbility(abilityIndexPairs[pairIndex].Key);
	const float radiusDistance = AbilitySystems::GetRadiusOfConstructionAbility(abilityIndexPairs[pairIndex].Key);

	FVector2D candidateOffset = (fromSinkToEntity * (safeZoneDistance + resourceSourceTransformComponent->m_radius + ArgusECSConstants::k_resourceSinkBufferDistanceAdjustment));
	FVector candidatePoint = FVector(candidateOffset, 0.0f) + resourceSourceTransformComponent->m_location;
	bool isBlocked = SpatialPartitioningSystems::AnyObstaclesOrStaticEntitiesInCircle(candidatePoint, radiusDistance, safeZoneDistance);
	if (isBlocked)
	{
		const int32 numIterations = 16;
		const float angleOffset = ArgusMath::SafeDivide(360.0f, static_cast<float>(numIterations));

		for (int32 i = 0; i < numIterations; ++i)
		{
			candidateOffset = candidateOffset.GetRotated(angleOffset);
			candidatePoint.X = resourceSourceTransformComponent->m_location.X + candidateOffset.X;
			candidatePoint.Y = resourceSourceTransformComponent->m_location.Y + candidateOffset.Y;

			if (!SpatialPartitioningSystems::AnyObstaclesOrStaticEntitiesInCircle(candidatePoint, radiusDistance, safeZoneDistance))
			{
				isBlocked = false;
				break;
			}
		}
	}

	if (isBlocked)
	{
		return false;
	}

	sources[extractionDataIndex].m_resourceSinkConstructorEntityId = entity.GetId();
	targetingComponent->SetLocationTarget(TransformSystems::ProjectLocationOntoNavigationData(worldReferenceComponent->m_worldPointer, radiusDistance, candidatePoint));
	return true;
}

ArgusEntity TeamCommanderSystems_AssignEntities::GetNearestSeenResourceSourceToEntity(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderComponent* teamCommanderComponent, EResourceType type, int32& outPairIndex, int32& outExtractionDataIndex)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, ArgusEntity::k_emptyEntity);

	outPairIndex = 0;

	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return ArgusEntity::k_emptyEntity;
	}

	ArgusEntity nearestResourceSource = ArgusEntity::k_emptyEntity;

	float minDistanceSquared = FLT_MAX;

	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> >& sources = teamCommanderComponent->GetSeenSourceExtractionDataForResourceType(type);
	for (int32 i = 0; i < sources.Num(); ++i)
	{
		if (sources[i].m_resourceSinkEntityId != ArgusECSConstants::k_maxEntities)
		{
			continue;
		}

		ArgusEntity resourceSourceEntity = ArgusEntity::RetrieveEntity(sources[i].m_resourceSourceEntityId);
		TransformComponent* resourceSinkSourceTransformComponent = resourceSourceEntity.GetComponent<TransformComponent>();
		if (!resourceSinkSourceTransformComponent)
		{
			continue;
		}

		bool anyValidAbilities = false;
		int32 j;
		for (j = 0; j < abilityIndexPairs.Num(); ++j)
		{
			if (ResourceSystems::CanEntityTemplateActAsSinkToEntitySource(AbilitySystems::GetEntityTemplateForAbility(abilityIndexPairs[j].Key), resourceSourceEntity))
			{
				anyValidAbilities = true;
				break;
			}
		}

		if (!anyValidAbilities)
		{
			continue;
		}

		const float distanceSquared = FVector::DistSquared2D(transformComponent->m_location, resourceSinkSourceTransformComponent->m_location);
		if (distanceSquared < minDistanceSquared)
		{
			minDistanceSquared = distanceSquared;
			nearestResourceSource = resourceSourceEntity;
			outPairIndex = j;
			outExtractionDataIndex = i;
		}
	}

	return nearestResourceSource;
}

