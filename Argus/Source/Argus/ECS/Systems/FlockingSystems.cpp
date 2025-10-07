// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FlockingSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

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
}

void FlockingSystems::EndFlockingIfNecessary(float deltaTime, const FlockingSystemsArgs& components)
{
	ARGUS_TRACE(FlockingSystems::EndFlockingIfNecessary);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const FVector targetLocation = components.m_entity.GetCurrentTargetLocation();
	const FVector currentLocation = components.m_transformComponent->m_location;
	const float distanceToTargetSquared = FVector::DistSquared2D(currentLocation, targetLocation);
	if (distanceToTargetSquared < FMath::Square(components.m_transformComponent->m_radius))
	{
		components.m_flockingComponent->m_flockingState = EFlockingState::Stable;
		return;
	}

	if (distanceToTargetSquared < components.m_flockingComponent->m_minDistanceFromFlockingPoint)
	{
		components.m_flockingComponent->m_minDistanceFromFlockingPoint = distanceToTargetSquared;
		components.m_flockingComponent->m_timeAtMinFlockingDistance = 0.0f;
	}
	else
	{
		components.m_flockingComponent->m_timeAtMinFlockingDistance += deltaTime;
	}

	if (components.m_flockingComponent->m_timeAtMinFlockingDistance > 0.5f)
	{
		components.m_flockingComponent->m_flockingState = EFlockingState::Stable;
		return;
	}
}