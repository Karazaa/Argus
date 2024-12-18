// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AvoidanceSystems.h"
#include "ArgusMath.h"
#include "ArgusSystemsManager.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"

static TAutoConsoleVariable<bool> CVarShowAvoidanceDebug(TEXT("Argus.Avoidance.ShowAvoidanceDebug"), false, TEXT(""));

void AvoidanceSystems::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(AvoidanceSystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		TransformSystems::TransformSystemsComponentArgs components;
		components.m_entity = potentialEntity;
		components.m_taskComponent = potentialEntity.GetComponent<TaskComponent>();
		components.m_transformComponent = potentialEntity.GetComponent<TransformComponent>();
		components.m_navigationComponent = potentialEntity.GetComponent<NavigationComponent>();
		components.m_targetingComponent = potentialEntity.GetComponent<TargetingComponent>();
		if (!components.m_entity || !components.m_taskComponent || !components.m_transformComponent ||
			!components.m_navigationComponent || !components.m_targetingComponent)
		{
			continue;
		}

		ProcessORCAvoidance(worldPointer, deltaTime, components);
	}
}

#pragma region Optimal Reciprocal Collision Avoidance
void AvoidanceSystems::ProcessORCAvoidance(UWorld* worldPointer, float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components)
{
	ARGUS_MEMORY_TRACE(ArgusAvoidanceSystems);
	ARGUS_TRACE(AvoidanceSystems::ProcessORCAvoidance);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId);
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

	CreateEntityORCALinesParams params;
	FVector sourceEntityLocation3D = components.m_transformComponent->m_transform.GetLocation();
	sourceEntityLocation3D.Z += k_debugVectorHeightAdjust;
	params.m_sourceEntityLocation = FVector2D(sourceEntityLocation3D);
	params.m_sourceEntityVelocity = FVector2D(components.m_transformComponent->m_currentVelocity);
	params.m_deltaTime = deltaTime;
	params.m_inversePredictionTime = 1.0f / ArgusECSConstants::k_avoidanceCollisionDetectionPredictionTime;
	FVector2D desiredVelocity = FVector2D::ZeroVector;

	// If we are moving, we need to get our desired velocity as the velocity that points towards the nearest pathing point at the desired speed.
	if (components.m_taskComponent->IsExecutingMoveTask())
	{
		const uint16 futureIndex = components.m_navigationComponent->m_lastPointIndex + 1u;
		const uint16 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.size();
		FVector desiredDirection = FVector::ZeroVector;
		if (numNavigationPoints != 0u && futureIndex < numNavigationPoints)
		{
			desiredDirection = (components.m_navigationComponent->m_navigationPoints[futureIndex] - components.m_transformComponent->m_transform.GetLocation());
		}
		else
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to process ORCA, but the source %s's %s is in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(NavigationComponent));
			desiredDirection = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
		}

		desiredVelocity = FVector2D(desiredDirection).GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
	}

	// Search for nearby entities within a specific range.
	std::vector<uint16> foundEntityIds;
	if (!spatialPartitioningComponent->m_argusKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(foundEntityIds, components.m_entity, ArgusECSConstants::k_avoidanceAgentSearchRadius))
	{
		// If no entities nearby, then nothing can effect our navigation, so we should just early out at our desired speed. 
		components.m_transformComponent->m_proposedAvoidanceVelocity = FVector(desiredVelocity, 0.0f);
		if (CVarShowAvoidanceDebug.GetValueOnGameThread() && worldPointer)
		{
			DrawDebugLine(worldPointer, sourceEntityLocation3D, sourceEntityLocation3D + components.m_transformComponent->m_proposedAvoidanceVelocity, FColor::Orange, false, -1.0f, 0, k_debugVectorWidth);
		}
		return;
	}

	// Iterate over the found entities and generate ORCA lines based on their current states.
	std::vector<ORCALine> calculatedORCALines;
	CreateObstacleORCALines(worldPointer, params, components, calculatedORCALines);
	const int numStaticObstacles = calculatedORCALines.size();
	CreateEntityORCALines(params, components, foundEntityIds, calculatedORCALines);

	int failureLine = -1;
	FVector2D resultingVelocity = FVector2D::ZeroVector;
	if (!TwoDimensionalLinearProgram(calculatedORCALines, components.m_transformComponent->m_desiredSpeedUnitsPerSecond, desiredVelocity, true, resultingVelocity, failureLine))
	{
		ThreeDimensionalLinearProgram(calculatedORCALines, components.m_transformComponent->m_desiredSpeedUnitsPerSecond, failureLine, numStaticObstacles, resultingVelocity);
	}

	if (resultingVelocity.SquaredLength() > FMath::Square(components.m_transformComponent->m_desiredSpeedUnitsPerSecond))
	{
		resultingVelocity = resultingVelocity.GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
	}

	if (CVarShowAvoidanceDebug.GetValueOnGameThread() && worldPointer)
	{
		DrawDebugLine(worldPointer, sourceEntityLocation3D, sourceEntityLocation3D + FVector(resultingVelocity, 0.0f), FColor::Orange, false, -1.0f, 0, k_debugVectorWidth);
		DrawDebugLine(worldPointer, sourceEntityLocation3D, sourceEntityLocation3D + FVector(desiredVelocity, 0.0f), FColor::Green, false, -1.0f, 0, k_debugVectorWidth);
	}

	components.m_transformComponent->m_proposedAvoidanceVelocity = FVector(resultingVelocity, 0.0f);
}

void AvoidanceSystems::CreateObstacleORCALines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, std::vector<ORCALine>& outORCALines)
{
	if (!worldPointer)
	{
		return;
	}

	TArray<FVector> foundNavEdges;
	RetrieveRelevantNavEdges(worldPointer, components, foundNavEdges);

	if (CVarShowAvoidanceDebug.GetValueOnGameThread())
	{
		FVector zAdjust = FVector(0.0f, 0.0f, 10.0f);
		for (int i = 0; i < foundNavEdges.Num(); i += 2)
		{
			DrawDebugLine(worldPointer, foundNavEdges[i] + zAdjust, foundNavEdges[i + 1] + zAdjust, FColor::Blue, false, -1.0f, 0u, k_debugVectorWidth);
		}
	}

	for (int i = 0; i < foundNavEdges.Num(); i += 2)
	{
		ORCALine calculatedORCALine;
		calculatedORCALine.m_point = FVector2D(foundNavEdges[0]);

		FVector2D relativeLocation = params.m_sourceEntityLocation - calculatedORCALine.m_point;
		FVector2D lineSegment = FVector2D(foundNavEdges[1] - foundNavEdges[0]);

		FVector2D potentialDirection0 = FVector2D(lineSegment.Y, -lineSegment.X);
		FVector2D potentialDirection1 = FVector2D(-lineSegment.Y, lineSegment.X);

		calculatedORCALine.m_direction = relativeLocation.Dot(potentialDirection0) ? potentialDirection0 : potentialDirection1;
		// outORCALines.push_back(calculatedORCALine);
	}
}

void AvoidanceSystems::CreateEntityORCALines(const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, std::vector<uint16>& foundEntityIds, std::vector<ORCALine>& outORCALines)
{
	for (int i = 0; i < foundEntityIds.size(); ++i)
	{
		ArgusEntity foundEntity = ArgusEntity::RetrieveEntity(foundEntityIds[i]);
		if (!foundEntity)
		{
			continue;
		}

		TransformComponent* foundTransformComponent = foundEntity.GetComponent<TransformComponent>();
		if (!foundTransformComponent)
		{
			continue;
		}

		CreateEntityORCALinesParamsPerEntity perEntityParams;
		perEntityParams.m_foundEntityLocation = FVector2D(foundTransformComponent->m_transform.GetLocation());
		perEntityParams.m_foundEntityVelocity = FVector2D(foundTransformComponent->m_currentVelocity);
		perEntityParams.m_entityRadius = components.m_transformComponent->m_radius;

		FVector2D velocityToBoundaryOfVO = FVector2D::ZeroVector;
		ORCALine calculatedORCALine;
		FindORCALineAndVelocityToBoundaryPerEntity(params, perEntityParams, velocityToBoundaryOfVO, calculatedORCALine);

		calculatedORCALine.m_point = params.m_sourceEntityVelocity + (velocityToBoundaryOfVO * GetEffortCoefficientForEntityPair(components, foundEntity));
		outORCALines.push_back(calculatedORCALine);
	}
}

void AvoidanceSystems::FindORCALineAndVelocityToBoundaryPerEntity(const CreateEntityORCALinesParams& params, const CreateEntityORCALinesParamsPerEntity& perEntityParams, FVector2D& velocityToBoundaryOfVO, ORCALine& calculatedORCALine)
{
	ARGUS_TRACE(AvoidanceSystems::FindORCALineAndVelocityToBoundaryPerEntity);

	const FVector2D relativeLocation = perEntityParams.m_foundEntityLocation - params.m_sourceEntityLocation;
	const FVector2D relativeVelocity = params.m_sourceEntityVelocity - perEntityParams.m_foundEntityVelocity;
	const float relativeLocationDistanceSquared = relativeLocation.SquaredLength();
	const float combinedRadius = 2.0f * perEntityParams.m_entityRadius;
	const float combinedRadiusSquared = FMath::Square(combinedRadius);

	if (relativeLocationDistanceSquared > combinedRadiusSquared)
	{
		// No collision yet.
		const FVector2D cutoffCenterToRelativeVelocity = relativeVelocity - (params.m_inversePredictionTime * relativeLocation);
		const float cutoffCenterToRelativeVelocityLengthSqared = cutoffCenterToRelativeVelocity.SquaredLength();
		const float dotProduct = cutoffCenterToRelativeVelocity.Dot(relativeLocation);

		if (dotProduct > 0.0f && FMath::Square(dotProduct) > combinedRadiusSquared * cutoffCenterToRelativeVelocityLengthSqared)
		{
			const float cutoffCenterToRelativeVelocityLength = FMath::Sqrt(cutoffCenterToRelativeVelocityLengthSqared);
			const FVector2D unitCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity / cutoffCenterToRelativeVelocityLength;
			calculatedORCALine.m_direction = FVector2D(unitCutoffCenterToRelativeVelocity.Y, -unitCutoffCenterToRelativeVelocity.X);
			velocityToBoundaryOfVO = ((combinedRadius * params.m_inversePredictionTime) - cutoffCenterToRelativeVelocityLength) * unitCutoffCenterToRelativeVelocity;
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

bool AvoidanceSystems::OneDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int lineIndex, FVector2D& resultingVelocity)
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

	for (int i = 0; i < lineIndex; ++i)
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

bool AvoidanceSystems::TwoDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int& failureLine)
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

	for (int i = 0; i < orcaLines.size(); ++i)
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

void AvoidanceSystems::ThreeDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const int lineIndex, const int numStaticObstacleORCALines, FVector2D& resultingVelocity)
{
	ARGUS_MEMORY_TRACE(ArgusAvoidanceSystems);

	float distance = 0.0f;

	for (int i = lineIndex; i < orcaLines.size(); ++i)
	{
		if (ArgusMath::Determinant(orcaLines[i].m_direction, (orcaLines[i].m_point - resultingVelocity)) <= distance)
		{
			continue;
		}

		std::vector<ORCALine> projectedLines(orcaLines.begin(), orcaLines.begin() + static_cast<std::ptrdiff_t>(numStaticObstacleORCALines));

		for (int j = numStaticObstacleORCALines; j < i; ++j)
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
			projectedLines.push_back(orcaLine);
		}

		const FVector2D cachedResultingVelocity = resultingVelocity;
		int failureLine = -1;
		if (!TwoDimensionalLinearProgram(projectedLines, radius, FVector2D(-orcaLines[i].m_direction.Y, orcaLines[i].m_direction.X), true, resultingVelocity, failureLine))
		{
			resultingVelocity = cachedResultingVelocity;
		}

		distance = ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[i].m_point - resultingVelocity);
	}
}

void AvoidanceSystems::RetrieveRelevantNavEdges(UWorld* worldPointer, const TransformSystems::TransformSystemsComponentArgs& components, TArray<FVector>& outNavEdges)
{
	ARGUS_MEMORY_TRACE(ArgusAvoidanceSystems);
	ARGUS_TRACE(AvoidanceSystems::RetrieveRelevantNavEdges);

	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in %s is nullptr."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer);
	if (!unrealNavigationSystem)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UNavigationSystemV1*));
		return;
	}

	ANavigationData* navData = unrealNavigationSystem->MainNavData;
	if (!unrealNavigationSystem)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ANavigationData*));
		return;
	}

	FVector entityLocation = components.m_transformComponent->m_transform.GetLocation();
	FNavLocation originLocation;
	if (!unrealNavigationSystem->ProjectPointToNavigation(entityLocation, originLocation))
	{
		return;
	}

	TArray<FVector> queryShapePoints;
	queryShapePoints.SetNumZeroed(4);
	queryShapePoints[0] = entityLocation;
	queryShapePoints[1] = entityLocation;
	queryShapePoints[2] = entityLocation;
	queryShapePoints[3] = entityLocation;

	queryShapePoints[0].X -= ArgusECSConstants::k_avoidanceAgentSearchRadius;
	queryShapePoints[1].X += ArgusECSConstants::k_avoidanceAgentSearchRadius;
	queryShapePoints[2].X += ArgusECSConstants::k_avoidanceAgentSearchRadius;
	queryShapePoints[3].X -= ArgusECSConstants::k_avoidanceAgentSearchRadius;
	queryShapePoints[0].Y += ArgusECSConstants::k_avoidanceAgentSearchRadius;
	queryShapePoints[1].Y += ArgusECSConstants::k_avoidanceAgentSearchRadius;
	queryShapePoints[2].Y -= ArgusECSConstants::k_avoidanceAgentSearchRadius;
	queryShapePoints[3].Y -= ArgusECSConstants::k_avoidanceAgentSearchRadius;

	navData->FindOverlappingEdges(originLocation, TConstArrayView<FVector>(queryShapePoints), outNavEdges);
}

float AvoidanceSystems::GetEffortCoefficientForEntityPair(const TransformSystems::TransformSystemsComponentArgs& sourceEntityComponents, const ArgusEntity& foundEntity)
{
	if (!sourceEntityComponents.m_entity || !sourceEntityComponents.m_taskComponent || !sourceEntityComponents.m_transformComponent ||
		!sourceEntityComponents.m_navigationComponent || !sourceEntityComponents.m_targetingComponent)
	{
		return 0.0f;
	}

	TaskComponent* foundEntityTaskComponent = foundEntity.GetComponent<TaskComponent>();
	if (!foundEntityTaskComponent)
	{
		return 1.0f;
	}

	IdentityComponent* foundEntityIdentityComponent = foundEntity.GetComponent<IdentityComponent>();
	if (!foundEntityIdentityComponent)
	{
		return 1.0f;
	}

	IdentityComponent* sourceEntityIdentityComponent = sourceEntityComponents.m_entity.GetComponent<IdentityComponent>();
	if (!sourceEntityIdentityComponent)
	{
		return 1.0f;
	}

	if (sourceEntityIdentityComponent->IsInTeamMask(foundEntityIdentityComponent->m_enemies))
	{
		return 1.0f;
	}

	if (sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() && (!foundEntityTaskComponent->IsExecutingMoveTask()))
	{
		return foundEntity.IsMoveable() ? 0.0f : 1.0f;
	}

	if (!sourceEntityComponents.m_taskComponent->IsExecutingMoveTask() && foundEntityTaskComponent->IsExecutingMoveTask())
	{
		return sourceEntityComponents.m_entity.IsMoveable() ? 1.0f : 0.0f;
	}

	return 0.5f;
}
#pragma endregion
