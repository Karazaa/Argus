// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AvoidanceSystems.h"

float AvoidanceSystems::GetEffortCoefficientForEntityPair(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const AvoidanceGroupingComponent* sourceGroupingComponent, const AvoidanceGroupingComponent* foundGroupingComponent, bool inSameAvoidanceGroup)
{
	ARGUS_TRACE(AvoidanceSystems::GetEffortCoefficientForEntityPair);

	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return 0.0f;
	}

	const TaskComponent* foundEntityTaskComponent = foundEntity.GetComponent<TaskComponent>();
	const IdentityComponent* foundEntityIdentityComponent = foundEntity.GetComponent<IdentityComponent>();
	const IdentityComponent* sourceEntityIdentityComponent = sourceEntityComponents.m_entity.GetComponent<IdentityComponent>();
	if (!foundEntityTaskComponent || !foundEntityIdentityComponent || !sourceEntityIdentityComponent)
	{
		return 1.0f;
	}

	if (sourceEntityIdentityComponent->IsInTeamMask(foundEntityIdentityComponent->m_enemies))
	{
		return sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() ? 1.0f : 0.0f;
	}

	float effortCoefficient = 0.5f;
	if (ShouldReturnMovabilityEffortCoefficient(sourceEntityComponents, foundEntity, effortCoefficient) ||
		ShouldReturnCombatEffortCoefficient(sourceEntityComponents, foundEntityTaskComponent, inSameAvoidanceGroup, effortCoefficient) ||
		ShouldReturnConstructionEffortCoefficient(sourceEntityComponents, foundEntityTaskComponent, effortCoefficient) ||
		ShouldReturnResourceExtractionEffortCoefficient(sourceEntityComponents, foundEntityTaskComponent, inSameAvoidanceGroup, effortCoefficient) ||
		ShouldReturnCarrierEffortCoefficient(sourceEntityComponents, foundEntity, foundEntityTaskComponent, effortCoefficient) ||
		ShouldReturnTargetEffortCoefficient(sourceEntityComponents, foundEntity, inSameAvoidanceGroup, effortCoefficient) ||
		ShouldReturnStaticFlockingEffortCoefficient(sourceEntityComponents, foundEntity, effortCoefficient) ||
		ShouldReturnAvoidancePriorityEffortCoefficient(sourceGroupingComponent, foundGroupingComponent, effortCoefficient) ||
		ShouldReturnMovementTaskEffortCoefficient(sourceEntityComponents, foundEntity, foundEntityTaskComponent, inSameAvoidanceGroup, effortCoefficient))
	{
		return effortCoefficient;
	}

	if (inSameAvoidanceGroup)
	{
		return GetEffortCoefficientForAvoidanceGroupPair(sourceEntityComponents, foundEntity, sourceGroupingComponent, foundGroupingComponent);
	}

	return 0.5f;
}

float AvoidanceSystems::GetEffortCoefficientForAvoidanceGroupPair(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const AvoidanceGroupingComponent* sourceGroupComponent, const AvoidanceGroupingComponent* foundGroupComponent)
{
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return 0.0f;
	}

	if (!sourceGroupComponent)
	{
		ARGUS_ERROR_NULL(ArgusECSLog, sourceGroupComponent);
		return 1.0f;
	}
	if (!foundGroupComponent)
	{
		ARGUS_ERROR_NULL(ArgusECSLog, foundGroupComponent);
		return 0.0f;
	}

	if (sourceGroupComponent->m_groupId != foundGroupComponent->m_groupId || sourceGroupComponent->m_groupId == ArgusECSConstants::k_maxEntities)
	{
		return 0.5f;
	}

	const TransformComponent* foundTransformComponent = foundEntity.GetComponent<TransformComponent>();
	if (!foundTransformComponent)
	{
		ARGUS_ERROR_NULL(ArgusECSLog, foundTransformComponent);
		return 0.0f;
	}

	const float squaredDistance = FVector::DistSquared2D(sourceEntityComponents.m_transformComponent->m_location, foundTransformComponent->m_location);
	if (squaredDistance > FMath::Square(sourceEntityComponents.m_transformComponent->m_radius * 2.0f))
	{
		return 0.0f;
	}

	return 0.5f;
}

bool AvoidanceSystems::ShouldReturnMovabilityEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, float& coefficient)
{
	if (!foundEntity.IsMoveable())
	{
		coefficient = 1.0f;
		return true;
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnCombatEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, bool inSameAvoidanceGroup, float& coefficient)
{
	ARGUS_RETURN_ON_NULL_BOOL(foundEntityTaskComponent, ArgusECSLog);
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (sourceEntityComponents.m_taskComponent->m_combatState == ECombatState::Attacking && foundEntityTaskComponent->m_combatState != ECombatState::Attacking)
	{
		coefficient = inSameAvoidanceGroup ? 0.33f : 0.0f;
		return true;
	}
	if (sourceEntityComponents.m_taskComponent->m_combatState != ECombatState::Attacking && foundEntityTaskComponent->m_combatState == ECombatState::Attacking)
	{
		coefficient = inSameAvoidanceGroup ? 0.67f : 1.0f;
		return true;
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnConstructionEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, float& coefficient)
{
	ARGUS_RETURN_ON_NULL_BOOL(foundEntityTaskComponent, ArgusECSLog);
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (sourceEntityComponents.m_taskComponent->m_constructionState == EConstructionState::ConstructingOther && foundEntityTaskComponent->m_constructionState != EConstructionState::ConstructingOther)
	{
		coefficient = 0.0f;
		return true;
	}
	if (sourceEntityComponents.m_taskComponent->m_constructionState != EConstructionState::ConstructingOther && foundEntityTaskComponent->m_constructionState == EConstructionState::ConstructingOther)
	{
		coefficient = 1.0f;
		return true;
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnCarrierEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const TaskComponent* foundEntityTaskComponent, float& coefficient)
{
	ARGUS_RETURN_ON_NULL_BOOL(foundEntityTaskComponent, ArgusECSLog);
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	const TargetingComponent* foundEntityTargetingComponent = foundEntity.GetComponent<TargetingComponent>();
	const PassengerComponent* sourceEntityPassengerComponent = sourceEntityComponents.m_entity.GetComponent<PassengerComponent>();
	const CarrierComponent* sourceEntityCarrierComponent = sourceEntityComponents.m_entity.GetComponent<CarrierComponent>();
	const PassengerComponent* foundEntityPassengerComponent = foundEntity.GetComponent<PassengerComponent>();
	const CarrierComponent* foundEntityCarrierComponent = foundEntity.GetComponent<CarrierComponent>();
	if (sourceEntityPassengerComponent &&
		foundEntityCarrierComponent &&
		sourceEntityComponents.m_targetingComponent->HasEntityTarget() &&
		sourceEntityComponents.m_targetingComponent->m_targetEntityId == foundEntity.GetId() &&
		sourceEntityComponents.m_taskComponent->m_movementState == EMovementState::MoveToEntity)
	{
		coefficient = 0.0f;
		return true;
	}

	if (foundEntityTargetingComponent &&
		sourceEntityCarrierComponent &&
		foundEntityPassengerComponent &&
		foundEntityTargetingComponent->HasEntityTarget() &&
		foundEntityTargetingComponent->m_targetEntityId == sourceEntityComponents.m_entity.GetId() &&
		foundEntityTaskComponent->m_movementState == EMovementState::MoveToEntity)
	{
		coefficient = 0.0f;
		return true;
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnTargetEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, bool inSameAvoidanceGroup, float& coefficient)
{
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (sourceEntityComponents.m_targetingComponent->HasEntityTarget() &&
		sourceEntityComponents.m_targetingComponent->m_targetEntityId == foundEntity.GetId() &&
		(sourceEntityComponents.m_taskComponent->m_movementState == EMovementState::MoveToEntity || sourceEntityComponents.m_taskComponent->m_movementState == EMovementState::InRangeOfTargetEntity))
	{
		coefficient = 0.0f;
		return true;
	}

	const TargetingComponent* foundEntityTargetingComponent = foundEntity.GetComponent<TargetingComponent>();
	const TaskComponent* foundEntityTaskComponent = foundEntity.GetComponent<TaskComponent>();
	if (!foundEntityTargetingComponent || !foundEntityTaskComponent)
	{
		return false;
	}

	if (foundEntityTargetingComponent->HasEntityTarget() &&
		foundEntityTargetingComponent->m_targetEntityId == sourceEntityComponents.m_entity.GetId() &&
		(foundEntityTaskComponent->m_movementState == EMovementState::MoveToEntity || foundEntityTaskComponent->m_movementState == EMovementState::InRangeOfTargetEntity))
	{
		coefficient = (inSameAvoidanceGroup || !sourceEntityComponents.m_taskComponent->IsExecutingMoveTask()) ? 0.0f : 1.0f;
		return true;
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnAvoidancePriorityEffortCoefficient(const AvoidanceGroupingComponent* sourceEntityAvoidanceGroupingComponent, const AvoidanceGroupingComponent* foundEntityAvoidanceGroupingComponent, float& coefficient)
{
	if (sourceEntityAvoidanceGroupingComponent && foundEntityAvoidanceGroupingComponent)
	{
		if (sourceEntityAvoidanceGroupingComponent->m_avoidancePriority > foundEntityAvoidanceGroupingComponent->m_avoidancePriority)
		{
			coefficient = 0.0f;
			return true;
		}

		if (sourceEntityAvoidanceGroupingComponent->m_avoidancePriority < foundEntityAvoidanceGroupingComponent->m_avoidancePriority)
		{
			coefficient = 1.0f;
			return true;
		}
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnMovementTaskEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const TaskComponent* foundEntityTaskComponent, bool inSameAvoidanceGroup, float& coefficient)
{
	ARGUS_RETURN_ON_NULL_BOOL(foundEntityTaskComponent, ArgusECSLog);
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() && (!foundEntityTaskComponent->IsExecutingMoveTask()))
	{
		coefficient = inSameAvoidanceGroup ? 1.0f : 0.0f;
		return true;
	}

	if (!sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() && foundEntityTaskComponent->IsExecutingMoveTask())
	{
		coefficient = inSameAvoidanceGroup ? 0.0f : 1.0f;
		return true;
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnResourceExtractionEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, bool inSameAvoidanceGroup, float& coefficient)
{
	ARGUS_RETURN_ON_NULL_BOOL(foundEntityTaskComponent, ArgusECSLog);
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (sourceEntityComponents.m_taskComponent->m_resourceExtractionState == EResourceExtractionState::Extracting && foundEntityTaskComponent->m_resourceExtractionState != EResourceExtractionState::Extracting)
	{
		coefficient = inSameAvoidanceGroup ? 0.33f : 0.0f;
		return true;
	}
	if (sourceEntityComponents.m_taskComponent->m_resourceExtractionState != EResourceExtractionState::Extracting && foundEntityTaskComponent->m_resourceExtractionState == EResourceExtractionState::Extracting)
	{
		coefficient = inSameAvoidanceGroup ? 0.67f : 1.0f;
		return true;
	}

	return false;
}

bool AvoidanceSystems::ShouldReturnStaticFlockingEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, float& coefficient)
{
	if (!sourceEntityComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	const FlockingComponent* sourceEntityFlockingComponent = sourceEntityComponents.m_entity.GetComponent<FlockingComponent>();
	const FlockingComponent* foundEntityFlockingComponent = foundEntity.GetComponent<FlockingComponent>();
	if (!sourceEntityFlockingComponent || !foundEntityFlockingComponent)
	{
		return false;
	}

	if (!sourceEntityComponents.m_entity.IsOnSameTeamAsOtherEntity(foundEntity))
	{
		return false;
	}

	TaskComponent* foundTaskComponent = foundEntity.GetComponent<TaskComponent>();
	TargetingComponent* foundTargetingComponent = foundEntity.GetComponent<TargetingComponent>();
	TransformComponent* foundTransformComponent = foundEntity.GetComponent<TransformComponent>();
	if (!foundTaskComponent || !foundTargetingComponent || !foundTransformComponent)
	{
		return false;
	}

	if (sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() || foundTaskComponent->IsExecutingMoveTask())
	{
		return false;
	}

	if (!sourceEntityComponents.m_targetingComponent->HasSameTarget(foundTargetingComponent))
	{
		return false;
	}

	if (sourceEntityFlockingComponent->m_flockingState == EFlockingState::Stable && foundEntityFlockingComponent->m_flockingState == EFlockingState::Stable)
	{
		return false;
	}

	if (sourceEntityFlockingComponent->m_flockingState == EFlockingState::Stable)
	{
		coefficient = 0.0f;
		return true;
	}

	if (foundEntityFlockingComponent->m_flockingState == EFlockingState::Stable)
	{
		coefficient = 1.0f;
		return true;
	}

	const float sourceDistSquared = FVector::DistSquared2D(sourceEntityComponents.m_transformComponent->m_location, sourceEntityComponents.m_entity.GetCurrentTargetLocation());
	const float targetDistSquared = FVector::DistSquared2D(foundTransformComponent->m_location, foundEntity.GetCurrentTargetLocation());
	coefficient = sourceDistSquared > targetDistSquared ? 1.0f : 0.0f;
	return true;
}