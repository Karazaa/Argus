// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FlockingSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/AvoidanceSystems.h"

void FlockingSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(FlockingSystems::RunSystems);

	FlockingSystemsArgs components;
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(i)))
		{
			continue;
		}

		// If executing move task or not shrinking, continue.
		if (components.m_taskComponent->IsExecutingMoveTask() || components.m_flockingComponent->m_flockingState != EFlockingState::Shrinking)
		{
			continue;
		}

		EndFlockingIfNecessary(deltaTime, components);
	}
} 

void FlockingSystems::ChooseFlockingRootEntityIfGroupLeader(const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	AvoidanceGroupingComponent* groupingComponent = components.m_entity.GetComponent<AvoidanceGroupingComponent>();
	FlockingComponent* flockingComponent = components.m_entity.GetComponent<FlockingComponent>();
	if (!groupingComponent)
	{
		return;
	}

	if (groupingComponent->m_groupId != components.m_entity.GetId())
	{
		flockingComponent->m_flockingRootId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!components.m_targetingComponent->HasLocationTarget())
	{
		flockingComponent->m_flockingRootId = ArgusECSConstants::k_maxEntities;
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter = [components](const ArgusEntityKDTreeNode* entityNode)
		{
			ARGUS_RETURN_ON_NULL_BOOL(entityNode, ArgusECSLog);
			if (!components.AreComponentsValidCheck(ARGUS_NAMEOF(TransformSystems::ChooseFlockingRootEntityIfGroupLeader)))
			{
				return false;
			}

			if (entityNode->m_entityId == components.m_entity.GetId())
			{
				return true;
			}

			ArgusEntity otherEntity = ArgusEntity::RetrieveEntity(entityNode->m_entityId);
			if (!otherEntity || otherEntity.IsPassenger())
			{
				return false;
			}

			if (!components.m_entity.IsOnSameTeamAsOtherEntity(otherEntity))
			{
				return false;
			}

			TargetingComponent* otherEntityTargetingComponent = otherEntity.GetComponent<TargetingComponent>();
			if (!otherEntityTargetingComponent)
			{
				return false;
			}

			return components.m_targetingComponent->HasSameTarget(otherEntityTargetingComponent);
		};

	flockingComponent->m_flockingRootId = spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdClosestToLocation(components.m_targetingComponent->m_targetLocation.GetValue(), queryFilter);
	if (flockingComponent->m_flockingRootId == ArgusECSConstants::k_maxEntities)
	{
		return;
	}

	ArgusEntity flockingRootEntity = ArgusEntity::RetrieveEntity(flockingComponent->m_flockingRootId);
	FlockingComponent* flockingRootFlockingComponent = flockingRootEntity.GetComponent<FlockingComponent>();
	ARGUS_RETURN_ON_NULL(flockingRootFlockingComponent, ArgusECSLog);

	flockingRootFlockingComponent->m_concentricFlockingTier = 1u;
	flockingRootFlockingComponent->m_numEntitiesInStableRange = 1u;
	flockingRootFlockingComponent->m_flockingState = EFlockingState::Stable;
}

ArgusEntity FlockingSystems::GetFlockingRootEntity(const ArgusEntity& entity)
{
	if (!entity)
	{
		return ArgusEntity::k_emptyEntity;
	}

	ArgusEntity groupLeader = AvoidanceSystems::GetAvoidanceGroupLeader(entity);
	if (!groupLeader)
	{
		return ArgusEntity::k_emptyEntity;
	}

	const FlockingComponent* flockingComponent = groupLeader.GetComponent<FlockingComponent>();
	if (!flockingComponent)
	{
		return ArgusEntity::k_emptyEntity;
	}

	return ArgusEntity::RetrieveEntity(flockingComponent->m_flockingRootId);
}

// Hex grid packing with a fallback timeout on shrinking duration.
void FlockingSystems::EndFlockingIfNecessary(float deltaTime, const FlockingSystemsArgs& components)
{
	ARGUS_TRACE(FlockingSystems::EndFlockingIfNecessary);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (PackFlockingRoot(components))
	{
		return;
	}

	// Timing based flocking
	const FVector targetLocation = components.m_entity.GetCurrentTargetLocation();
	const FVector currentLocation = components.m_transformComponent->m_location;
	const float distanceToTargetSquared = FVector::DistSquared2D(currentLocation, targetLocation);

	if (distanceToTargetSquared < components.m_flockingComponent->m_minDistanceFromFlockingPoint)
	{
		components.m_flockingComponent->m_minDistanceFromFlockingPoint = distanceToTargetSquared;
		components.m_flockingComponent->m_timeAtMinFlockingDistance = 0.0f;
	}
	else
	{
		components.m_flockingComponent->m_timeAtMinFlockingDistance += deltaTime;
	}

	if (components.m_flockingComponent->m_timeAtMinFlockingDistance > components.m_flockingComponent->m_maxShrinkingDurationTimeoutSeconds)
	{
		components.m_flockingComponent->m_flockingState = EFlockingState::Stable;
		return;
	}
}

// Hex grid based packing.
bool FlockingSystems::PackFlockingRoot(const FlockingSystemsArgs& components)
{
	ARGUS_TRACE(FlockingSystems::PackFlockingRoot);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	ArgusEntity flockingRootEntity = GetFlockingRootEntity(components.m_entity);
	if (!flockingRootEntity)
	{
		return false;
	}

	FlockingComponent* flockingRootFlockingComponent = flockingRootEntity.GetComponent<FlockingComponent>();
	const TransformComponent* flockingRootTransformComponent = flockingRootEntity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(flockingRootFlockingComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL_BOOL(flockingRootTransformComponent, ArgusECSLog);

	const float distanceToRootSquared = FVector::DistSquared2D(flockingRootTransformComponent->m_location, components.m_transformComponent->m_location);
	const float flockingRootRadiusSquared = FMath::Square(GetCurrentFlockingRootRadius(flockingRootFlockingComponent));

	if (distanceToRootSquared > flockingRootRadiusSquared)
	{
		return false;
	}

	const uint16 currentMaxCount = GetCurrentFlockingRootMaxCount(flockingRootFlockingComponent);
	components.m_flockingComponent->m_flockingState = EFlockingState::Stable;
	flockingRootFlockingComponent->m_numEntitiesInStableRange++;

	if (flockingRootFlockingComponent->m_numEntitiesInStableRange >= currentMaxCount)
	{
		flockingRootFlockingComponent->m_concentricFlockingTier++;
	}

	return true;
}

float FlockingSystems::GetCurrentFlockingRootRadius(const FlockingComponent* flockingRootFlockingComponent)
{
	ARGUS_RETURN_ON_NULL_FLOAT(flockingRootFlockingComponent, ArgusECSLog, 0.0f);

	if (flockingRootFlockingComponent->m_concentricFlockingTier == 0u)
	{
		return flockingRootFlockingComponent->m_flockingRootRadiusIncrement;
	}

	return flockingRootFlockingComponent->m_concentricFlockingTier * 2.0f * flockingRootFlockingComponent->m_flockingRootRadiusIncrement;
}

uint16 FlockingSystems::GetCurrentFlockingRootMaxCount(const FlockingComponent* flockingRootFlockingComponent)
{
	ARGUS_RETURN_ON_NULL_UINT16(flockingRootFlockingComponent, ArgusECSLog, 1u);

	if (flockingRootFlockingComponent->m_concentricFlockingTier == 0u)
	{
		return 1u;
	}

	uint16 output = 1u;
	for (uint16 i = 0u; i < flockingRootFlockingComponent->m_concentricFlockingTier; ++i)
	{
		output += ((i + 1u) * 6u);
	}

	return output;
}
