// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FlockingSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/TargetingSystems.h"

void FlockingSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(FlockingSystems::RunSystems);

	ClearPackingValues();
	SetPackingValues();
	SetFlockingState(deltaTime);
} 

ArgusEntity FlockingSystems::GetFlockingRootEntity(ArgusEntity entity)
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

	FlockingComponent* flockingComponent = groupLeader.GetComponent<FlockingComponent>();
	if (!flockingComponent)
	{
		return ArgusEntity::k_emptyEntity;
	}

	return groupLeader;
}

FlockingComponent* FlockingSystems::GetFlockingRootComponent(ArgusEntity entity)
{
	if (ArgusEntity rootEntity = GetFlockingRootEntity(entity))
	{
		return rootEntity.GetComponent<FlockingComponent>();
	}

	return nullptr;
}

FVector FlockingSystems::GetFlockingPoint(ArgusEntity flockingRootEntity)
{
	const AvoidanceGroupingComponent* flockingRootGroupingComponent = flockingRootEntity.GetComponent<AvoidanceGroupingComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(flockingRootGroupingComponent, ArgusECSLog, FVector::ZeroVector);

	if (flockingRootGroupingComponent->m_numberOfIdleEntities > 0)
	{
		const TargetingComponent* flockingRootTargetingComponent = flockingRootEntity.GetComponent<TargetingComponent>();
		const TransformComponent* flockingRootTransformComponent = flockingRootEntity.GetComponent<TransformComponent>();
		ARGUS_RETURN_ON_NULL_VALUE(flockingRootTargetingComponent, ArgusECSLog, flockingRootGroupingComponent->m_groupAverageLocation);
		ARGUS_RETURN_ON_NULL_VALUE(flockingRootTransformComponent, ArgusECSLog, flockingRootGroupingComponent->m_groupAverageLocation);

		if (flockingRootTargetingComponent->HasAnyTarget())
		{
			return flockingRootEntity.GetCurrentTargetLocation();
		}

		return flockingRootTransformComponent->m_location;
	}

	return flockingRootGroupingComponent->m_groupAverageLocation;
}


void FlockingSystems::ClearPackingValues()
{
	ARGUS_TRACE(FlockingSystems::ClearPackingValues);

	ArgusEntity::IterateEntities([](ArgusEntity entity)
	{
		if (FlockingComponent* flockingComponent = entity.GetComponent<FlockingComponent>())
		{
			flockingComponent->ResetPackingValues();
		}
	});
}

void FlockingSystems::SetPackingValues()
{
	ARGUS_TRACE(FlockingSystems::SetPackingValues);

	ArgusEntity::IterateSystemsArgs<FlockingSystemsArgs>([](FlockingSystemsArgs& components)
	{
		if (components.m_flockingComponent->m_flockingState != EFlockingState::Stable)
		{
			return;
		}

		if (FlockingComponent* flockingRootComponent = GetFlockingRootComponent(components.m_entity))
		{
			IncrementStableEntitiesInRange(flockingRootComponent);
		}
	});
}

void FlockingSystems::SetFlockingState(float deltaTime)
{
	ARGUS_TRACE(FlockingSystems::SetFlockingState);

	ArgusEntity::IterateSystemsArgs<FlockingSystemsArgs>([deltaTime](FlockingSystemsArgs& components)
	{
		if (components.m_flockingComponent->m_flockingState == EFlockingState::Shrinking)
		{
			EndFlockingIfNecessary(deltaTime, components);
		}
		else
		{
			StartFlockingIfNecessary(components);
		}
	});
}

void FlockingSystems::StartFlockingIfNecessary(const FlockingSystemsArgs& components)
{
	ARGUS_TRACE(FlockingSystems::StartFlockingIfNecessary);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	ArgusEntity flockingRootEntity = GetFlockingRootEntity(components.m_entity);
	if (!flockingRootEntity)
	{
		return;
	}

	FlockingComponent* flockingRootFlockingComponent = flockingRootEntity.GetComponent<FlockingComponent>();
	ARGUS_RETURN_ON_NULL(flockingRootFlockingComponent, ArgusECSLog);
	
	const float distanceToRootSquared = FVector::DistSquared2D(GetFlockingPoint(flockingRootEntity), components.m_transformComponent->m_location);
	const float flockingRootRadiusSquared = FMath::Square(GetCurrentFlockingRootRadius(flockingRootFlockingComponent));

	if (distanceToRootSquared > flockingRootRadiusSquared)
	{
		components.m_flockingComponent->m_flockingState = EFlockingState::Shrinking;
	}
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

	if (!components.m_targetingComponent->HasAnyTarget())
	{
		components.m_flockingComponent->ResetTimingValues();
		components.m_flockingComponent->m_flockingState = EFlockingState::Stable;
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
		components.m_flockingComponent->ResetTimingValues();
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
	ARGUS_RETURN_ON_NULL_BOOL(flockingRootFlockingComponent, ArgusECSLog);

	const float distanceToRootSquared = FVector::DistSquared2D(GetFlockingPoint(flockingRootEntity), components.m_transformComponent->m_location);
	const float flockingRootRadiusSquared = FMath::Square(GetCurrentFlockingRootRadius(flockingRootFlockingComponent));

	if (distanceToRootSquared > flockingRootRadiusSquared)
	{
		return false;
	}

	components.m_flockingComponent->ResetTimingValues();
	components.m_flockingComponent->m_flockingState = EFlockingState::Stable;
	return true;
}

float FlockingSystems::GetCurrentFlockingRootRadius(const FlockingComponent* flockingRootFlockingComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(flockingRootFlockingComponent, ArgusECSLog, 0.0f);

	if (flockingRootFlockingComponent->m_concentricFlockingTier == 0u)
	{
		return flockingRootFlockingComponent->m_flockingRootRadiusIncrement;
	}

	return flockingRootFlockingComponent->m_concentricFlockingTier * 2.0f * flockingRootFlockingComponent->m_flockingRootRadiusIncrement;
}

uint16 FlockingSystems::GetCurrentFlockingRootMaxCount(const FlockingComponent* flockingRootFlockingComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(flockingRootFlockingComponent, ArgusECSLog, 1u);

	return GetFlockingRootMaxCountForTier(flockingRootFlockingComponent->m_concentricFlockingTier);
}

uint16 FlockingSystems::GetFlockingRootMaxCountForTier(uint8 flockingTier)
{
	uint16 output = 1u;
	for (uint8 i = 0u; i < flockingTier; ++i)
	{
		output += ((i + 1u) * 6u);
	}

	return output;
}

void FlockingSystems::IncrementStableEntitiesInRange(FlockingComponent* flockingRootFlockingComponent)
{
	ARGUS_RETURN_ON_NULL(flockingRootFlockingComponent, ArgusECSLog);

	const uint16 currentMaxCount = GetCurrentFlockingRootMaxCount(flockingRootFlockingComponent);
	flockingRootFlockingComponent->m_numEntitiesInStableRange++;

	if (flockingRootFlockingComponent->m_numEntitiesInStableRange >= currentMaxCount)
	{
		flockingRootFlockingComponent->m_concentricFlockingTier++;
	}
}
