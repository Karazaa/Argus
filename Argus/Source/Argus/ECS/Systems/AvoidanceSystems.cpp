// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AvoidanceSystems.h"
#include "ArgusMath.h"
#include "ArgusSystemsManager.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "Systems/CombatSystems.h"
#include "Systems/TargetingSystems.h"
#include <limits>

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

void AvoidanceSystems::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(AvoidanceSystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		TransformSystems::TransformSystemsComponentArgs components;
		components.m_entity = ArgusEntity::RetrieveEntity(i);
		if (!components.m_entity)
		{
			continue;
		}

		if (components.m_entity.IsKillable() && !components.m_entity.IsAlive())
		{
			continue;
		}

		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_transformComponent = components.m_entity.GetComponent<TransformComponent>();
		components.m_navigationComponent = components.m_entity.GetComponent<NavigationComponent>();
		components.m_targetingComponent = components.m_entity.GetComponent<TargetingComponent>();
		if (!components.m_entity || !components.m_taskComponent || !components.m_transformComponent ||
			!components.m_navigationComponent || !components.m_targetingComponent)
		{
			continue;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			continue;
		}

		const AvoidanceGroupingComponent* avoidanceGroupingComponent = components.m_entity.GetComponent<AvoidanceGroupingComponent>();
		if (!avoidanceGroupingComponent)
		{
			continue;
		}

		ProcessORCAvoidance(worldPointer, deltaTime, components, avoidanceGroupingComponent);
	}
}

#pragma region Optimal Reciprocal Collision Avoidance
void AvoidanceSystems::ProcessORCAvoidance(UWorld* worldPointer, float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components, const AvoidanceGroupingComponent* avoidanceGroupingComponent)
{
	ARGUS_MEMORY_TRACE(ArgusAvoidanceSystems);
	ARGUS_TRACE(AvoidanceSystems::ProcessORCAvoidance);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !avoidanceGroupingComponent)
	{
		return;
	}

	const ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve singleton %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity))
		return;
	}

	const SpatialPartitioningComponent* const spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpatialPartitioningComponent*))
		return;
	}

	FVector2D desiredVelocity = GetDesiredVelocity(components);

	CreateEntityORCALinesParams params;
	params.m_sourceEntityLocation3D = components.m_transformComponent->m_location;
	params.m_sourceEntityLocation3D.Z += ArgusECSConstants::k_debugDrawHeightAdjustment;
	params.m_sourceEntityLocation = ArgusMath::ToCartesianVector2(FVector2D(params.m_sourceEntityLocation3D));
	params.m_sourceEntityVelocity = ArgusMath::ToCartesianVector2(FVector2D(components.m_transformComponent->m_currentVelocity));
	params.m_deltaTime = deltaTime;
	params.m_entityRadius = components.m_transformComponent->m_radius;
	params.m_inverseEntityPredictionTime = 1.0f / ArgusECSConstants::k_avoidanceEntityDetectionPredictionTime;
	params.m_inverseObstaclePredictionTime = 1.0f / ArgusECSConstants::k_avoidanceObstacleDetectionPredictionTime;
	params.m_spatialPartitioningComponent = spatialPartitioningComponent;

	// If no entities nearby, then nothing can effect our navigation, so we should just early out with a desired velocity. 
	const TArray<uint16>& foundEntityIds = avoidanceGroupingComponent->m_adjacentEntities;
	if (foundEntityIds.IsEmpty())
	{
		// If we have no desired velocity, we should try to get a desired velocity direction towards where we last navigated too.
		if (!components.m_taskComponent->IsExecutingMoveTask())
		{
			desiredVelocity = GetVelocityTowardsEndOfNavPoint(params, components);
		}

		components.m_transformComponent->m_proposedAvoidanceVelocity = FVector(ArgusMath::ToUnrealVector2(desiredVelocity), 0.0f);
#if !UE_BUILD_SHIPPING
		if (worldPointer && ArgusECSDebugger::ShouldShowAvoidanceDebugForEntity(components.m_entity.GetId()))
		{
			DrawDebugLine(worldPointer, params.m_sourceEntityLocation3D, params.m_sourceEntityLocation3D + components.m_transformComponent->m_proposedAvoidanceVelocity, FColor::Orange, false, -1.0f, 0, ArgusECSConstants::k_debugDrawLineWidth);
		}
#endif //!UE_BUILD_SHIPPING
		return;
	}

	// Iterate over the found entities and generate ORCA lines based on their current states.
	TArray<ORCALine> calculatedORCALines;
	CreateObstacleORCALines(worldPointer, params, components, calculatedORCALines);
#if !UE_BUILD_SHIPPING
	if (worldPointer && ArgusECSDebugger::ShouldShowAvoidanceDebugForEntity(components.m_entity.GetId()))
	{
		DrawORCADebugLines(worldPointer, params, calculatedORCALines, true, 0);
	}
#endif //!UE_BUILD_SHIPPING
	const int32 numStaticObstacles = calculatedORCALines.Num();
	
	CreateEntityORCALines(params, components, foundEntityIds, calculatedORCALines, desiredVelocity);

	int32 failureLine = -1;
	FVector2D resultingVelocity = FVector2D::ZeroVector;
	if (!TwoDimensionalLinearProgram(calculatedORCALines, components.m_transformComponent->m_desiredSpeedUnitsPerSecond, desiredVelocity, true, resultingVelocity, failureLine))
	{
		ThreeDimensionalLinearProgram(calculatedORCALines, components.m_transformComponent->m_desiredSpeedUnitsPerSecond, failureLine, numStaticObstacles, resultingVelocity);
	}

	if (resultingVelocity.SquaredLength() > FMath::Square(components.m_transformComponent->m_desiredSpeedUnitsPerSecond))
	{
		resultingVelocity = resultingVelocity.GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
	}

#if !UE_BUILD_SHIPPING
	if (worldPointer && ArgusECSDebugger::ShouldShowAvoidanceDebugForEntity(components.m_entity.GetId()))
	{
		DrawORCADebugLines(worldPointer, params, calculatedORCALines, false, numStaticObstacles);
		DrawDebugLine(worldPointer, params.m_sourceEntityLocation3D, params.m_sourceEntityLocation3D + FVector(ArgusMath::ToUnrealVector2(resultingVelocity), 0.0f), FColor::Magenta, false, -1.0f, 0, ArgusECSConstants::k_debugDrawLineWidth);
		DrawDebugLine(worldPointer, params.m_sourceEntityLocation3D, params.m_sourceEntityLocation3D + FVector(ArgusMath::ToUnrealVector2(desiredVelocity), 0.0f), FColor::Turquoise, false, -1.0f, 0, ArgusECSConstants::k_debugDrawLineWidth);
	}
#endif //!UE_BUILD_SHIPPING

	components.m_transformComponent->m_proposedAvoidanceVelocity = FVector(ArgusMath::ToUnrealVector2(resultingVelocity), 0.0f);
}

ArgusEntity AvoidanceSystems::GetAvoidanceGroupLeader(const ArgusEntity& entity)
{
	if (!entity)
	{
		return ArgusEntity::k_emptyEntity;
	}

	const AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>();
	if (!avoidanceGroupingComponent)
	{
		return ArgusEntity::k_emptyEntity;
	}

	return ArgusEntity::RetrieveEntity(avoidanceGroupingComponent->m_groupId);
}

bool AvoidanceSystems::AreInSameAvoidanceGroup(const ArgusEntity& entity, const ArgusEntity& otherEntity)
{
	const AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>();
	if (!avoidanceGroupingComponent)
	{
		return false;
	}

	const AvoidanceGroupingComponent* otherAvoidanceGroupingComponent = otherEntity.GetComponent<AvoidanceGroupingComponent>();
	if (!otherAvoidanceGroupingComponent)
	{
		return false;
	}

	return avoidanceGroupingComponent->m_groupId == otherAvoidanceGroupingComponent->m_groupId;
}

void AvoidanceSystems::DecrementIdleEntitiesInGroup(const ArgusEntity& entity)
{
	if (!entity)
	{
		return;
	}

	ArgusEntity groupLeader = GetAvoidanceGroupLeader(entity);
	if (!groupLeader)
	{
		return;
	}

	if (AvoidanceGroupingComponent* groupingComponent = groupLeader.GetComponent<AvoidanceGroupingComponent>())
	{
		if (entity.IsIdle())
		{
			groupingComponent->m_numberOfIdleEntities--;
		}
	}
}

TOptional<FVector> AvoidanceSystems::GetAvoidanceGroupDestinationLocation(const TransformSystems::TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return NullOpt;
	}

	const AvoidanceGroupingComponent* avoidanceGroupingComponent = components.m_entity.GetComponent<AvoidanceGroupingComponent>();
	if (!avoidanceGroupingComponent)
	{
		return NullOpt;
	}

	ArgusEntity groupLeaderEntity = ArgusEntity::RetrieveEntity(avoidanceGroupingComponent->m_groupId);
	if (!groupLeaderEntity)
	{
		return NullOpt;
	}

	TOptional<FVector> groupDestinationLocation = TargetingSystems::GetCurrentTargetLocationForEntity(components.m_entity);
	if (!groupDestinationLocation.IsSet())
	{
		return NullOpt;
	}

	return groupDestinationLocation;
}

TOptional<FVector> AvoidanceSystems::GetAvoidanceGroupSourceLocation(const TransformSystems::TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return NullOpt;
	}

	const AvoidanceGroupingComponent* avoidanceGroupingComponent = components.m_entity.GetComponent<AvoidanceGroupingComponent>();
	if (!avoidanceGroupingComponent)
	{
		return NullOpt;
	}

	ArgusEntity groupLeaderEntity = ArgusEntity::RetrieveEntity(avoidanceGroupingComponent->m_groupId);
	if (!groupLeaderEntity)
	{
		return NullOpt;
	}

	const AvoidanceGroupingComponent* groupLeaderAvoidanceGroupingComponent = groupLeaderEntity.GetComponent<AvoidanceGroupingComponent>();
	if (!groupLeaderAvoidanceGroupingComponent)
	{
		return NullOpt;
	}

	TOptional<FVector> output = TOptional<FVector>(groupLeaderAvoidanceGroupingComponent->m_groupAverageLocation);
	return output;
}

void AvoidanceSystems::CreateObstacleORCALines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, TArray<ORCALine>& outORCALines)
{
	if (!worldPointer || !params.m_spatialPartitioningComponent)
	{
		return;
	}

	const float adjacentEntityRange = components.m_transformComponent->m_desiredSpeedUnitsPerSecond + components.m_transformComponent->m_radius;
	const float obstacleQueryRange = components.m_transformComponent->m_radius * ArgusECSConstants::k_avoidanceObstacleQueryRadiusMultiplier;

#if !UE_BUILD_SHIPPING
	if (ArgusECSDebugger::ShouldShowAvoidanceDebugForEntity(components.m_entity.GetId()))
	{
		DrawDebugCircle(worldPointer, params.m_sourceEntityLocation3D, adjacentEntityRange, 20, FColor::Yellow, false, -1.0f, 0, ArgusECSConstants::k_debugDrawLineWidth, FVector::RightVector, FVector::ForwardVector, false);
	}
#endif //!UE_BUILD_SHIPPING
	
	TArray<ObstacleIndicies> obstacleIndicies;
	params.m_spatialPartitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation
	(
		obstacleIndicies,
		FVector(params.m_sourceEntityLocation, components.m_transformComponent->m_location.Z),
		obstacleQueryRange
	);

	for (int32 i = 0; i < obstacleIndicies.Num(); ++i)
	{
		const ObstaclePoint& previous = params.m_spatialPartitioningComponent->m_obstacles[obstacleIndicies[i].m_obstacleIndex].GetPrevious(obstacleIndicies[i].m_obstaclePointIndex);
		const ObstaclePoint& current = params.m_spatialPartitioningComponent->m_obstacles[obstacleIndicies[i].m_obstacleIndex][obstacleIndicies[i].m_obstaclePointIndex];
		const ObstaclePoint& next = params.m_spatialPartitioningComponent->m_obstacles[obstacleIndicies[i].m_obstacleIndex].GetNext(obstacleIndicies[i].m_obstaclePointIndex);

		CalculateORCALineForObstacleSegment(params, current, next, previous.m_direction, outORCALines);

#if !UE_BUILD_SHIPPING
		if (ArgusECSDebugger::ShouldShowAvoidanceDebugForEntity(components.m_entity.GetId()))
		{
			previous.DrawDebugObstaclePoint(worldPointer);
			current.DrawDebugObstaclePoint(worldPointer);
			next.DrawDebugObstaclePoint(worldPointer);
		}
#endif //!UE_BUILD_SHIPPING
	}
}

void AvoidanceSystems::CreateEntityORCALines(const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, const TArray<uint16>& foundEntityIds, TArray<ORCALine>& outORCALines, FVector2D& outDesiredVelocity)
{
	const bool calculateAverageLocationOfOtherEntities = outDesiredVelocity.IsNearlyZero() && !params.m_sourceEntityVelocity.IsNearlyZero();
	FVector2D averageLocationOfOtherEntities = FVector2D::ZeroVector;
	float numberOfEntitiesInAverage = 0.0f;

	for (int32 i = 0; i < foundEntityIds.Num(); ++i)
	{
		ArgusEntity foundEntity = ArgusEntity::RetrieveEntity(foundEntityIds[i]);
		if (!foundEntity)
		{
			continue;
		}

		if ((foundEntity.IsKillable() && !foundEntity.IsAlive()) || foundEntity.IsPassenger())
		{
			continue;
		}

		TransformComponent* foundTransformComponent = foundEntity.GetComponent<TransformComponent>();
		if (!foundTransformComponent)
		{
			continue;
		}

		const float effortCoefficient = GetEffortCoefficientForEntityPair(components, foundEntity);
		if (effortCoefficient == 0.0f)
		{
			continue;
		}

		CreateEntityORCALinesParamsPerEntity perEntityParams;
		perEntityParams.m_foundEntityLocation = ArgusMath::ToCartesianVector2(FVector2D(foundTransformComponent->m_location));
		perEntityParams.m_foundEntityVelocity = ArgusMath::ToCartesianVector2(FVector2D(foundTransformComponent->m_currentVelocity));
		perEntityParams.m_entityRadius = foundTransformComponent->m_radius;

		if (calculateAverageLocationOfOtherEntities)
		{
			const float bufferRadius = ArgusECSConstants::k_avoidanceAgentAdditionalBufferRadius + components.m_transformComponent->m_radius + perEntityParams.m_entityRadius;
			if (!AreInSameAvoidanceGroup(components.m_entity, foundEntity) && FVector2D::DistSquared(params.m_sourceEntityLocation, perEntityParams.m_foundEntityLocation) < FMath::Square(bufferRadius))
			{
				numberOfEntitiesInAverage += 1.0f;
				averageLocationOfOtherEntities += perEntityParams.m_foundEntityLocation + (perEntityParams.m_foundEntityVelocity * ArgusECSConstants::k_avoidanceEntityDetectionPredictionTime);
			}
		}

		FVector2D velocityToBoundaryOfVO = FVector2D::ZeroVector;
		ORCALine calculatedORCALine;
		FindORCALineAndVelocityToBoundaryPerEntity(params, perEntityParams, velocityToBoundaryOfVO, calculatedORCALine);

		calculatedORCALine.m_point = params.m_sourceEntityVelocity + (velocityToBoundaryOfVO * effortCoefficient);
		outORCALines.Add(calculatedORCALine);
	}

	if (!calculateAverageLocationOfOtherEntities || numberOfEntitiesInAverage == 0.0f)
	{
		return;
	}

	averageLocationOfOtherEntities /= numberOfEntitiesInAverage;
	outDesiredVelocity = (params.m_sourceEntityLocation - averageLocationOfOtherEntities).GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
}

void AvoidanceSystems::FindORCALineAndVelocityToBoundaryPerEntity(const CreateEntityORCALinesParams& params, const CreateEntityORCALinesParamsPerEntity& perEntityParams, FVector2D& velocityToBoundaryOfVO, ORCALine& calculatedORCALine)
{
	ARGUS_TRACE(AvoidanceSystems::FindORCALineAndVelocityToBoundaryPerEntity);

	const FVector2D relativeLocation = perEntityParams.m_foundEntityLocation - params.m_sourceEntityLocation;
	const FVector2D relativeVelocity = params.m_sourceEntityVelocity - perEntityParams.m_foundEntityVelocity;
	const float relativeLocationDistanceSquared = relativeLocation.SquaredLength();
	const float combinedRadius = params.m_entityRadius + perEntityParams.m_entityRadius;
	const float combinedRadiusSquared = FMath::Square(combinedRadius);

	if (relativeLocationDistanceSquared > combinedRadiusSquared)
	{
		// This appears to work... for reasons... I just observed that the ORCA direction appeared inverted for the trailing entity in a set moving in roughly the same direction.
		const bool invertDirection = params.m_sourceEntityVelocity.Dot(perEntityParams.m_foundEntityVelocity) > 0.0f && params.m_sourceEntityVelocity.Dot(relativeLocation) > 0.0f;

		// No collision yet.
		const FVector2D cutoffCenterToRelativeVelocity = relativeVelocity - (params.m_inverseEntityPredictionTime * relativeLocation);
		const float cutoffCenterToRelativeVelocityLengthSqared = cutoffCenterToRelativeVelocity.SquaredLength();
		const float dotProduct = cutoffCenterToRelativeVelocity.Dot(relativeLocation);

		if ((dotProduct < 0.0f) && (FMath::Square(dotProduct) > (combinedRadiusSquared * cutoffCenterToRelativeVelocityLengthSqared)))
		{
			const float cutoffCenterToRelativeVelocityLength = FMath::Sqrt(cutoffCenterToRelativeVelocityLengthSqared);
			const FVector2D unitCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity / cutoffCenterToRelativeVelocityLength;
			calculatedORCALine.m_direction = FVector2D(unitCutoffCenterToRelativeVelocity.Y, -unitCutoffCenterToRelativeVelocity.X);
			if (invertDirection)
			{
				calculatedORCALine.m_direction *= -1.0f;
			}
			velocityToBoundaryOfVO = ((combinedRadius * params.m_inverseEntityPredictionTime) - cutoffCenterToRelativeVelocityLength) * unitCutoffCenterToRelativeVelocity;
		}
		else
		{
			const float leg = FMath::Sqrt(relativeLocationDistanceSquared - combinedRadiusSquared);
			if (ArgusMath::Determinant(relativeLocation, cutoffCenterToRelativeVelocity) > 0.0f)
			{
				calculatedORCALine.m_direction = FVector2D((relativeLocation.X * leg) - (relativeLocation.Y * combinedRadius),
					(relativeLocation.X * combinedRadius) + (relativeLocation.Y * leg));
			}
			else
			{
				calculatedORCALine.m_direction = -FVector2D((relativeLocation.X * leg) + (relativeLocation.Y * combinedRadius),
					(-relativeLocation.X * combinedRadius) + (relativeLocation.Y * leg));
			}
			calculatedORCALine.m_direction /= relativeLocationDistanceSquared;
			velocityToBoundaryOfVO = (relativeVelocity.Dot(calculatedORCALine.m_direction) * calculatedORCALine.m_direction) - relativeVelocity;
		}
	}
	else
	{
		// Collision occurred.
		const float inverseDeltaTime = 1.0f / params.m_deltaTime;

		const FVector2D cutoffCenterToRelativeVelocity = relativeVelocity - (inverseDeltaTime * relativeLocation);
		const float lengthCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity.Length();
		const FVector2D normalizedCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity / lengthCutoffCenterToRelativeVelocity;

		calculatedORCALine.m_direction = FVector2D(normalizedCutoffCenterToRelativeVelocity.Y, -normalizedCutoffCenterToRelativeVelocity.X);
		velocityToBoundaryOfVO = ((combinedRadius * inverseDeltaTime) - lengthCutoffCenterToRelativeVelocity) * normalizedCutoffCenterToRelativeVelocity;
	}
}

bool AvoidanceSystems::OneDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int32 lineIndex, FVector2D& resultingVelocity)
{
	ARGUS_TRACE(AvoidanceSystems::OneDimensionalLinearProgram);

	const float dotProduct = orcaLines[lineIndex].m_point.Dot(orcaLines[lineIndex].m_direction);
	const float discriminant = FMath::Square(dotProduct) + FMath::Square(radius) - orcaLines[lineIndex].m_point.SquaredLength();

	if (discriminant < 0.0f)
	{
		return false;
	}

	const float sqrtDiscriminant = FMath::Sqrt(discriminant);
	float tLeft = -dotProduct - sqrtDiscriminant;
	float tRight = -dotProduct + sqrtDiscriminant;

	for (int32 i = 0; i < lineIndex; ++i)
	{
		const float denominator = ArgusMath::Determinant(orcaLines[lineIndex].m_direction, orcaLines[i].m_direction);
		const float numerator = ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[lineIndex].m_point - orcaLines[i].m_point);

		if (FMath::IsNearlyZero(denominator, ArgusECSConstants::k_avoidanceEpsilonValue))
		{
			if (numerator < 0.0f)
			{
				return false;
			}

			continue;
		}

		const float t = numerator / denominator;

		if (denominator >= 0.0f)
		{
			tRight = FMath::Min(tRight, t);
		}
		else
		{
			tLeft = FMath::Max(tLeft, t);
		}

		if (tLeft > tRight)
		{
			return false;
		}
	}

	if (shouldOptimizeDirection)
	{
		if (preferredVelocity.Dot(orcaLines[lineIndex].m_direction) > 0.0f)
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tRight * orcaLines[lineIndex].m_direction);
		}
		else
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tLeft * orcaLines[lineIndex].m_direction);
		}
	}
	else
	{
		const float t = orcaLines[lineIndex].m_direction.Dot((preferredVelocity - orcaLines[lineIndex].m_point));
		if (t < tLeft)
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tLeft * orcaLines[lineIndex].m_direction);
		}
		else if (t < tRight)
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tRight * orcaLines[lineIndex].m_direction);
		}
		else
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (t * orcaLines[lineIndex].m_direction);
		}
	}

	return true;
}

bool AvoidanceSystems::TwoDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int32& failureLine)
{
	ARGUS_TRACE(AvoidanceSystems::TwoDimensionalLinearProgram);

	if (shouldOptimizeDirection)
	{
		resultingVelocity = preferredVelocity * radius;
	}
	else if (preferredVelocity.SquaredLength() > FMath::Square(radius))
	{
		resultingVelocity = preferredVelocity.GetSafeNormal() * radius;
	}
	else
	{
		resultingVelocity = preferredVelocity;
	}

	for (int32 i = 0; i < orcaLines.Num(); ++i)
	{
		if (ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[i].m_point - resultingVelocity) > 0.0f)
		{
			const FVector2D cachedResultingVelocity = resultingVelocity;
			if (!OneDimensionalLinearProgram(orcaLines, radius, preferredVelocity, shouldOptimizeDirection, i, resultingVelocity))
			{
				resultingVelocity = resultingVelocity;
				failureLine = i;
				return false;
			}
		}
	}

	failureLine = -1;
	return true;
}

void AvoidanceSystems::ThreeDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const int32 lineIndex, const int numStaticObstacleORCALines, FVector2D& resultingVelocity)
{
	ARGUS_MEMORY_TRACE(ArgusAvoidanceSystems);

	float distance = 0.0f;

	for (int32 i = lineIndex; i < orcaLines.Num(); ++i)
	{
		if (ArgusMath::Determinant(orcaLines[i].m_direction, (orcaLines[i].m_point - resultingVelocity)) <= distance)
		{
			continue;
		}

		TArray<ORCALine> projectedLines;
		projectedLines.Append(&orcaLines[0], numStaticObstacleORCALines);

		for (int32 j = numStaticObstacleORCALines; j < i; ++j)
		{
			ORCALine orcaLine;
			const float determinant = ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[j].m_direction);
			if (FMath::IsNearlyZero(determinant, ArgusECSConstants::k_avoidanceEpsilonValue))
			{
				if (orcaLines[j].m_direction.Dot(orcaLines[j].m_direction) > 0.0f)
				{
					continue;
				}

				orcaLine.m_point = 0.5f * (orcaLines[i].m_point + orcaLines[j].m_point);
			}
			else
			{
				orcaLine.m_point = orcaLines[i].m_point + ((ArgusMath::Determinant(orcaLines[j].m_direction, orcaLines[i].m_point - orcaLines[j].m_point) / determinant) * orcaLines[i].m_direction);
			}

			orcaLine.m_direction = (orcaLines[j].m_direction - orcaLines[i].m_direction).GetSafeNormal();
			projectedLines.Add(orcaLine);
		}

		const FVector2D cachedResultingVelocity = resultingVelocity;
		int32 failureLine = -1;
		if (!TwoDimensionalLinearProgram(projectedLines, radius, FVector2D(-orcaLines[i].m_direction.Y, orcaLines[i].m_direction.X), true, resultingVelocity, failureLine))
		{
			resultingVelocity = cachedResultingVelocity;
		}

		distance = ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[i].m_point - resultingVelocity);
	}
}

FVector2D AvoidanceSystems::GetVelocityTowardsEndOfNavPoint(const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components)
{
	const FVector2D endedNavigationLocation = ArgusMath::ToCartesianVector2(FVector2D(components.m_navigationComponent->m_endedNavigationLocation));
	const FVector2D towardsEndNavigationLocation = endedNavigationLocation - params.m_sourceEntityLocation;
	float squareDistance = towardsEndNavigationLocation.SquaredLength();

	if (squareDistance > FMath::Square(ArgusECSConstants::k_avoidanceAgentReturnToEndNavRadius))
	{
		return towardsEndNavigationLocation.GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
	}

	return FVector2D::ZeroVector;
}

FVector2D AvoidanceSystems::GetDesiredVelocity(const TransformSystems::TransformSystemsComponentArgs& components)
{
	// If we are not executing a move task, we would like to early out with zero velocity as our desired velocity (this may change as we define more functionality for AvoidanceGroups)
	if (!components.m_taskComponent->IsExecutingMoveTask())
	{
		return FVector2D::ZeroVector;
	}

	// If our group leader is stopped, we should come to a stop as well.
	ArgusEntity groupLeader = GetAvoidanceGroupLeader(components.m_entity);
	if (groupLeader)
	{
		AvoidanceGroupingComponent* groupLeaderAvoidanceGroupingComponent = groupLeader.GetComponent<AvoidanceGroupingComponent>();
		if (groupLeaderAvoidanceGroupingComponent && groupLeaderAvoidanceGroupingComponent->m_numberOfIdleEntities > 0u)
		{
			components.m_taskComponent->m_movementState = EMovementState::AwaitingFinish;
			components.m_transformComponent->m_currentVelocity = FVector::ZeroVector;
			return FVector2D::ZeroVector;
		}
	}

	// If we are moving, we need to get our desired velocity as the velocity that points towards the nearest pathing point at the desired speed.
	const int32 futureIndex = components.m_navigationComponent->m_lastPointIndex + 1;
	const int32 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.Num();
	FVector desiredDirection = FVector::ZeroVector;
	if (numNavigationPoints != 0u && futureIndex < numNavigationPoints)
	{
		TOptional<FVector> sourceLocation = GetAvoidanceGroupSourceLocation(components);
		if (!sourceLocation.IsSet())
		{
			sourceLocation = components.m_transformComponent->m_location;
		}
		desiredDirection = (components.m_navigationComponent->m_navigationPoints[futureIndex] - sourceLocation.GetValue());
	}
	else
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to process ORCA, but the source %s's %s is in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(NavigationComponent));
		desiredDirection = ArgusMath::GetDirectionFromYaw(components.m_transformComponent->GetCurrentYaw());
	}

	return ArgusMath::ToCartesianVector2(FVector2D(desiredDirection).GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond);
}

float AvoidanceSystems::GetEffortCoefficientForEntityPair(const TransformSystems::TransformSystemsComponentArgs& sourceEntityComponents, const ArgusEntity& foundEntity)
{
	ARGUS_TRACE(AvoidanceSystems::GetEffortCoefficientForEntityPair);

	if (!sourceEntityComponents.m_entity || !sourceEntityComponents.m_taskComponent || !sourceEntityComponents.m_transformComponent ||
		!sourceEntityComponents.m_navigationComponent || !sourceEntityComponents.m_targetingComponent)
	{
		return 0.0f;
	}

	if (sourceEntityComponents.m_taskComponent->m_combatState == ECombatState::Attacking)
	{
		return 0.0f;
	}

	if (sourceEntityComponents.m_taskComponent->m_constructionState == EConstructionState::ConstructingOther && sourceEntityComponents.m_targetingComponent->HasEntityTarget())
	{
		if (const TransformComponent* targetedEntityTransformComponent = ArgusEntity::RetrieveEntity(sourceEntityComponents.m_targetingComponent->m_targetEntityId).GetComponent<TransformComponent>())
		{
			const float squaredDistance = FVector::DistSquared(targetedEntityTransformComponent->m_location, sourceEntityComponents.m_transformComponent->m_location);

			if (squaredDistance < FMath::Square(sourceEntityComponents.m_targetingComponent->m_meleeRange))
			{
				return 0.0f;
			}
		}
	}

	const TaskComponent* foundEntityTaskComponent = foundEntity.GetComponent<TaskComponent>();
	if (!foundEntityTaskComponent)
	{
		return 1.0f;
	}

	const IdentityComponent* foundEntityIdentityComponent = foundEntity.GetComponent<IdentityComponent>();
	if (!foundEntityIdentityComponent)
	{
		return 1.0f;
	}

	const IdentityComponent* sourceEntityIdentityComponent = sourceEntityComponents.m_entity.GetComponent<IdentityComponent>();
	if (!sourceEntityIdentityComponent)
	{
		return 1.0f;
	}

	if (sourceEntityIdentityComponent->IsInTeamMask(foundEntityIdentityComponent->m_enemies))
	{
		return sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() ? 1.0f : 0.0f;
	}

	if (foundEntityTaskComponent->m_combatState == ECombatState::Attacking)
	{
		return 1.0f;
	}

	const TargetingComponent* foundEntityTargetingComponent = foundEntity.GetComponent<TargetingComponent>();
	if (foundEntityTaskComponent->m_constructionState == EConstructionState::ConstructingOther)
	{
		const TransformComponent* foundEntityTransformComponent = foundEntity.GetComponent<TransformComponent>();
		const TransformComponent* foundEntityTargetTransformComponent = nullptr;

		if (foundEntityTransformComponent && foundEntityTargetingComponent && foundEntityTargetingComponent->HasEntityTarget())
		{
			foundEntityTargetTransformComponent = ArgusEntity::RetrieveEntity(foundEntityTargetingComponent->m_targetEntityId).GetComponent<TransformComponent>();
		}

		if (foundEntityTargetTransformComponent)
		{
			const float squaredDistance = FVector::DistSquared(foundEntityTargetTransformComponent->m_location, foundEntityTransformComponent->m_location);

			if (squaredDistance < FMath::Square(foundEntityTargetingComponent->m_meleeRange))
			{
				return 1.0f;
			}
		}
	}

	const TargetingComponent* sourceEntityTargetingComponent = sourceEntityComponents.m_entity.GetComponent<TargetingComponent>();
	const PassengerComponent* sourceEntityPassengerComponent = sourceEntityComponents.m_entity.GetComponent<PassengerComponent>();
	const CarrierComponent* sourceEntityCarrierComponent = sourceEntityComponents.m_entity.GetComponent<CarrierComponent>();
	const PassengerComponent* foundEntityPassengerComponent = foundEntity.GetComponent<PassengerComponent>();
	const CarrierComponent* foundEntityCarrierComponent = foundEntity.GetComponent<CarrierComponent>();
	if (sourceEntityTargetingComponent &&
		sourceEntityPassengerComponent &&
		foundEntityCarrierComponent &&
		sourceEntityTargetingComponent->HasEntityTarget() &&
		sourceEntityTargetingComponent->m_targetEntityId == foundEntity.GetId() &&
		sourceEntityComponents.m_taskComponent->m_movementState == EMovementState::MoveToEntity)
	{
		return 0.0f;
	}
	else if (foundEntityTargetingComponent &&
		sourceEntityCarrierComponent &&
		foundEntityPassengerComponent &&
		foundEntityTargetingComponent->HasEntityTarget() &&
		foundEntityTargetingComponent->m_targetEntityId == sourceEntityComponents.m_entity.GetId() &&
		foundEntityTaskComponent->m_movementState == EMovementState::MoveToEntity)
	{
		return 0.0f;
	}

	if (sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() && (!foundEntityTaskComponent->IsExecutingMoveTask()))
	{
		if (foundEntity.IsMoveable())
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
	}

	if (!sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() && foundEntityTaskComponent->IsExecutingMoveTask())
	{
		if (sourceEntityComponents.m_entity.IsMoveable())
		{
			return AreInSameAvoidanceGroup(sourceEntityComponents.m_entity, foundEntity) ? 0.0f : 1.0f;
		}
		else
		{
			return 0.0f;
		}
	}

	return 0.5f;
}

float AvoidanceSystems::FindAreaOfObstacleCartesian(const TArray<ObstaclePoint>& obstaclePoints)
{
	float area = 0.0f;

	for (int32 i = 0; i < obstaclePoints.Num(); ++i)
	{
		FVector2D point0 = obstaclePoints[i].m_point;
		FVector2D point1 = obstaclePoints[(i + 1) % obstaclePoints.Num()].m_point;

		float width = point1.X - point0.X;
		float height = (point1.Y + point0.Y) / 2.0f;

		area += width * height;
	}

	return area;
}

void AvoidanceSystems::CalculateORCALineForObstacleSegment(const CreateEntityORCALinesParams& params, ObstaclePoint obstaclePoint0, ObstaclePoint obstaclePoint1, const FVector2D& previousObstaclePointDir, TArray<ORCALine>& outORCALines)
{
	const FVector2D relativeLocation0 = obstaclePoint0.m_point - params.m_sourceEntityLocation;
	const FVector2D relativeLocation1 = obstaclePoint1.m_point - params.m_sourceEntityLocation;

	// Check if the velocity obstacle of the obstacle is already covered by existing ORCA lines.
	for (int32 i = 0; i < outORCALines.Num(); ++i)
	{
		const float determinant0 = ArgusMath::Determinant(params.m_inverseObstaclePredictionTime * (relativeLocation0 - outORCALines[i].m_point), outORCALines[i].m_direction);
		const float determinant1 = ArgusMath::Determinant(params.m_inverseObstaclePredictionTime * (relativeLocation1 - outORCALines[i].m_point), outORCALines[i].m_direction);
		const float scaledRadius = params.m_inverseObstaclePredictionTime * params.m_entityRadius;
		const bool point0Check = (determinant0 - scaledRadius) >= -ArgusECSConstants::k_avoidanceEpsilonValue;
		const bool point1Check = (determinant1 - scaledRadius) >= -ArgusECSConstants::k_avoidanceEpsilonValue;

		if (point0Check && point1Check)
		{
			return;
		}
	}

	const float squaredDistance0 = relativeLocation0.SquaredLength();
	const float squaredDistance1 = relativeLocation1.SquaredLength();
	const float radiusSquared = FMath::Square(params.m_entityRadius);
	const FVector2D obstacleVector = obstaclePoint1.m_point - obstaclePoint0.m_point;
	const float sValue = obstacleVector.Dot(-relativeLocation0) / obstacleVector.SquaredLength();
	const float squaredDistanceLine = (-relativeLocation0 - (sValue * obstacleVector)).SquaredLength();

	ORCALine line;

	// Collision with left vertex, Ignore if non convex.
	if (sValue < 0.0f && squaredDistance0 <= radiusSquared)
	{
		if (obstaclePoint0.m_isConvex)
		{
			line.m_point = FVector2D::ZeroVector;
			line.m_direction = FVector2D((-relativeLocation0).Y, relativeLocation0.X).GetSafeNormal();
			outORCALines.Add(line);
			return;
		}
	}

	// Collision with right vertex, Ignore if non convex or if it will be taken care of by neighboring obstacle
	if (sValue > 1.0f && squaredDistance1 <= radiusSquared)
	{
		if (obstaclePoint1.m_isConvex && ArgusMath::Determinant(relativeLocation1, obstaclePoint1.m_direction) >= 0.0f)
		{
			line.m_point = FVector2D::ZeroVector;
			line.m_direction = FVector2D((-relativeLocation1).Y, relativeLocation1.X).GetSafeNormal();
			outORCALines.Add(line);
			return;
		}
	}

	// Collision with obstacle segment
	if (sValue >= 0.0f && sValue <= 1.0f && squaredDistanceLine <= radiusSquared)
	{
		line.m_point = FVector2D::ZeroVector;
		line.m_direction = -obstaclePoint0.m_direction;
		outORCALines.Add(line);
		return;
	}

	// No collision. Compute legs. When obliquely viewed, both legs can come
	// from a single vertex. Legs extend cut-off line when nonconvex vertex.
	FVector2D leftLegDirection;
	FVector2D rightLegDirection;

	if (sValue < 0.0f && squaredDistanceLine <= radiusSquared)
	{
		if (!obstaclePoint0.m_isConvex)
		{
			return;
		}

		obstaclePoint1 = obstaclePoint0;

		const float leg0 = FMath::Sqrt(squaredDistance0 - radiusSquared);
		leftLegDirection = FVector2D
		(
			(relativeLocation0.X * leg0) - (relativeLocation0.Y * params.m_entityRadius),
			(relativeLocation0.X * params.m_entityRadius) + (relativeLocation0.Y * leg0)
		) / squaredDistance0;
		rightLegDirection = FVector2D
		(
			(relativeLocation0.X * leg0) + (relativeLocation0.Y * params.m_entityRadius),
			(-relativeLocation0.X * params.m_entityRadius) + (relativeLocation0.Y * leg0)
		) / squaredDistance0;
	}
	else if (sValue > 1.0f && squaredDistanceLine <= radiusSquared)
	{
		if (!obstaclePoint1.m_isConvex)
		{
			return;
		}

		obstaclePoint0 = obstaclePoint1;

		const float leg1 = FMath::Sqrt(squaredDistance1 - radiusSquared);
		leftLegDirection = FVector2D
		(
			(relativeLocation1.X * leg1) - (relativeLocation1.Y * params.m_entityRadius),
			(relativeLocation1.X * params.m_entityRadius) + (relativeLocation1.Y * leg1)
		) / squaredDistance1;
		rightLegDirection = FVector2D
		(
			(relativeLocation1.X * leg1) + (relativeLocation1.Y * params.m_entityRadius),
			(-relativeLocation1.X * params.m_entityRadius) + (relativeLocation1.Y * leg1)
		) / squaredDistance1;
	}
	else
	{
		if (obstaclePoint0.m_isConvex)
		{
			const float leg0 = FMath::Sqrt(squaredDistance0 - radiusSquared);
			leftLegDirection = FVector2D
			(
				(relativeLocation0.X * leg0) - (relativeLocation0.Y * params.m_entityRadius),
				(relativeLocation0.X * params.m_entityRadius) + (relativeLocation0.Y * leg0)
			) / squaredDistance0;
		}
		else
		{
			// Left vertex non-convex; left leg extends cut-off line.
			leftLegDirection = -obstaclePoint0.m_direction;
		}

		if (obstaclePoint1.m_isConvex)
		{
			const float leg1 = FMath::Sqrt(squaredDistance1 - radiusSquared);
			rightLegDirection = FVector2D
			(
				(relativeLocation1.X * leg1) + (relativeLocation1.Y * params.m_entityRadius),
				(-relativeLocation1.X * params.m_entityRadius) + (relativeLocation1.Y * leg1)
			) / squaredDistance1;
		}
		else
		{
			// Right vertex non-convex; right leg extends cut-off line.
			rightLegDirection = obstaclePoint0.m_direction;
		}
	}

	bool isLeftLegForeign = false;
	bool isRightLegForeign = false;

	if (obstaclePoint0.m_isConvex && ArgusMath::Determinant(leftLegDirection, -previousObstaclePointDir) >= 0.0f)
	{
		// Left leg points into obstacle
		leftLegDirection = -previousObstaclePointDir;
		isLeftLegForeign = true;
	}

	if (obstaclePoint1.m_isConvex && ArgusMath::Determinant(rightLegDirection, obstaclePoint1.m_direction) <= 0.0f)
	{
		// Right leg points into obstacle
		rightLegDirection = obstaclePoint1.m_direction;
		isRightLegForeign = true;
	}

	// Compute cutoff centers
	const FVector2D leftCutoff = params.m_inverseObstaclePredictionTime * (obstaclePoint0.m_point - params.m_sourceEntityLocation);
	const FVector2D rightCutoff = params.m_inverseObstaclePredictionTime * (obstaclePoint1.m_point - params.m_sourceEntityLocation);
	const FVector2D cutoffVector = rightCutoff - leftCutoff;

	// Project current velocity onto velocity obstacle
	
	// Check if current velocity is projected on cutoff circles
	const bool areObstaclesEqual =	obstaclePoint0.m_point == obstaclePoint1.m_point && 
									obstaclePoint0.m_direction == obstaclePoint1.m_direction && 
									obstaclePoint0.m_isConvex == obstaclePoint1.m_isConvex;
	const float tValue = areObstaclesEqual ? 0.5f : (params.m_sourceEntityVelocity - leftCutoff).Dot(cutoffVector) / cutoffVector.SquaredLength();
	const float tLeft = (params.m_sourceEntityVelocity - leftCutoff).Dot(leftLegDirection);
	const float tRight = (params.m_sourceEntityVelocity - rightCutoff).Dot(rightLegDirection);

	if ((tValue < 0.0f && tLeft < 0.0f) || (areObstaclesEqual && tLeft < 0.0f && tRight < 0.0f))
	{
		// Project onto left cutoff circle
		const FVector2D unitW = (params.m_sourceEntityVelocity - leftCutoff).GetSafeNormal();
		line.m_direction = FVector2D(unitW.Y, -unitW.X);
		line.m_point = leftCutoff + (params.m_entityRadius * params.m_inverseObstaclePredictionTime * unitW);
		outORCALines.Add(line);
		return;
	}

	if (tValue > 1.0f && tRight < 0.0f)
	{
		// Project onto right cutoff circle
		const FVector2D unitW = (params.m_sourceEntityVelocity - rightCutoff).GetSafeNormal();
		line.m_direction = FVector2D(unitW.Y, -unitW.X);
		line.m_point = rightCutoff + (params.m_entityRadius * params.m_inverseObstaclePredictionTime * unitW);
		outORCALines.Add(line);
		return;
	}

	// Project on left leg, right leg, or cut - off line, whichever is closest to velocity
	const float squaredDistanceCutoff = (tValue < 0.0f || tValue > 1.0f || areObstaclesEqual) ? 
		std::numeric_limits<float>::infinity() : (params.m_sourceEntityVelocity - (leftCutoff + (tValue * cutoffVector))).SquaredLength();
	const float squaredDistanceLeft = tLeft < 0.0f ?
		std::numeric_limits<float>::infinity() : (params.m_sourceEntityVelocity - (leftCutoff + (tLeft * leftLegDirection))).SquaredLength();
	const float squaredDistanceRight= tRight < 0.0f ?
		std::numeric_limits<float>::infinity() : (params.m_sourceEntityVelocity - (rightCutoff + (tRight * rightLegDirection))).SquaredLength();

	if (squaredDistanceCutoff <= squaredDistanceLeft && squaredDistanceCutoff <= squaredDistanceRight)
	{
		// Project on cutoff line
		line.m_direction = -obstaclePoint0.m_direction;
		line.m_point = leftCutoff + (params.m_entityRadius * params.m_inverseObstaclePredictionTime * FVector2D(-line.m_direction.Y, line.m_direction.X));
		outORCALines.Add(line);
		return;
	}

	if (squaredDistanceLeft <= squaredDistanceRight)
	{
		if (isLeftLegForeign)
		{
			return;
		}

		line.m_direction = leftLegDirection;
		line.m_point = leftCutoff + (params.m_entityRadius * params.m_inverseObstaclePredictionTime * FVector2D(-line.m_direction.Y, line.m_direction.X));
		outORCALines.Add(line);
		return;
	}

	if (isRightLegForeign)
	{
		return;
	}

	line.m_direction = -rightLegDirection;
	line.m_point = rightCutoff + (params.m_entityRadius * params.m_inverseObstaclePredictionTime * FVector2D(-line.m_direction.Y, line.m_direction.X));
	outORCALines.Add(line);
}

void AvoidanceSystems::DrawORCADebugLines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TArray<ORCALine>& orcaLines, bool areObstacleLines, int32 startingLine)
{
	if (!worldPointer)
	{
		return;
	}

	const FVector relativeVelocityBasisTranslation = FVector(ArgusMath::ToUnrealVector2(params.m_sourceEntityLocation), 0.0f);
	const FQuat relativeVelocityBasisRotation = FRotationMatrix::MakeFromXZ(FVector::ForwardVector, FVector::UpVector).ToQuat();
	const FTransform basisTransform = FTransform(relativeVelocityBasisRotation, relativeVelocityBasisTranslation);

	FColor debugColor = areObstacleLines ? FColor::Purple : FColor::Cyan;

	for (int32 i = startingLine; i < orcaLines.Num(); ++i)
	{
		const FVector worldspacePoint = basisTransform.TransformPosition(FVector(ArgusMath::ToUnrealVector2(orcaLines[i].m_point), 0.0f));
		const FVector worldspaceDirection = basisTransform.TransformVector(FVector(ArgusMath::ToUnrealVector2(orcaLines[i].m_direction), 0.0f));
		const FVector worldspaceOrthogonalDirectionScaled = worldspaceDirection.Cross(FVector::UpVector) * 1000.0f;

		DrawDebugSphere(worldPointer, worldspacePoint, 10.0f, 10u, debugColor, false, -1.0f, 0u, ArgusECSConstants::k_debugDrawLineWidth);
		DrawDebugLine(worldPointer, worldspacePoint, worldspacePoint + (worldspaceDirection * 100.0f), FColor::Red, false, -1.0f, 0u, ArgusECSConstants::k_debugDrawLineWidth);
		DrawDebugLine(worldPointer, worldspacePoint, worldspacePoint + (worldspaceDirection * -100.0f), FColor::Green, false, -1.0f, 0u, ArgusECSConstants::k_debugDrawLineWidth);
		DrawDebugLine(worldPointer, worldspacePoint - worldspaceOrthogonalDirectionScaled, worldspacePoint + worldspaceOrthogonalDirectionScaled, debugColor, false, -1.0f, 0u, ArgusECSConstants::k_debugDrawLineWidth);
	}
}

#pragma endregion
