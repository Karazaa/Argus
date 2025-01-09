// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AvoidanceSystems.h"
#include "ArgusMath.h"
#include "ArgusSystemsManager.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include <limits>

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
	params.m_sourceEntityLocation3D = components.m_transformComponent->m_transform.GetLocation();
	params.m_sourceEntityLocation3D.Z += k_debugVectorHeightAdjust;
	params.m_sourceEntityLocation = ArgusMath::ToCartesianVector2(FVector2D(params.m_sourceEntityLocation3D));
	params.m_sourceEntityVelocity = ArgusMath::ToCartesianVector2(FVector2D(components.m_transformComponent->m_currentVelocity));
	params.m_deltaTime = deltaTime;
	params.m_entityRadius = components.m_transformComponent->m_radius;
	params.m_inverseEntityPredictionTime = 1.0f / ArgusECSConstants::k_avoidanceEntityDetectionPredictionTime;
	params.m_inverseObstaclePredictionTime = 1.0f / ArgusECSConstants::k_avoidanceObstacleDetectionPredictionTime;
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

		desiredVelocity = ArgusMath::ToCartesianVector2(FVector2D(desiredDirection).GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond);
	}

	// Search for nearby entities within a specific range.
	std::vector<uint16> foundEntityIds;
	if (!spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(foundEntityIds, components.m_entity, ArgusECSConstants::k_avoidanceAgentSearchRadius))
	{
		// If no entities nearby, then nothing can effect our navigation, so we should just early out at our desired speed. 
		components.m_transformComponent->m_proposedAvoidanceVelocity = FVector(ArgusMath::ToUnrealVector2(desiredVelocity), 0.0f);
		if (CVarShowAvoidanceDebug.GetValueOnGameThread() && worldPointer)
		{
			DrawDebugLine(worldPointer, params.m_sourceEntityLocation3D, params.m_sourceEntityLocation3D + components.m_transformComponent->m_proposedAvoidanceVelocity, FColor::Orange, false, -1.0f, 0, k_debugVectorWidth);
		}
		return;
	}

	// Iterate over the found entities and generate ORCA lines based on their current states.
	std::vector<ORCALine> calculatedORCALines;
	CreateObstacleORCALines(worldPointer, params, components, calculatedORCALines);
	if (CVarShowAvoidanceDebug.GetValueOnGameThread() && worldPointer)
	{
		DrawORCADebugLines(worldPointer, params, calculatedORCALines, true, 0);
	}
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
		DrawORCADebugLines(worldPointer, params, calculatedORCALines, false, numStaticObstacles);
		DrawDebugLine(worldPointer, params.m_sourceEntityLocation3D, params.m_sourceEntityLocation3D + FVector(ArgusMath::ToUnrealVector2(resultingVelocity), 0.0f), FColor::Orange, false, -1.0f, 0, k_debugVectorWidth);
		DrawDebugLine(worldPointer, params.m_sourceEntityLocation3D, params.m_sourceEntityLocation3D + FVector(ArgusMath::ToUnrealVector2(desiredVelocity), 0.0f), FColor::Green, false, -1.0f, 0, k_debugVectorWidth);
	}

	components.m_transformComponent->m_proposedAvoidanceVelocity = FVector(ArgusMath::ToUnrealVector2(resultingVelocity), 0.0f);
}

void AvoidanceSystems::CreateObstacleORCALines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, std::vector<ORCALine>& outORCALines)
{
	if (!worldPointer)
	{
		return;
	}

	TArray<FVector> foundNavEdges;
	TArray<TArray<ObstaclePoint>> obstacles;
	RetrieveRelevantNavEdges(worldPointer, components, foundNavEdges);
	CalculateObstacles(params.m_sourceEntityLocation, foundNavEdges, obstacles);

	for (int32 i = 0; i < obstacles.Num(); ++i)
	{
		for (int32 j = 0; j < obstacles[i].Num(); ++j)
		{
			if (CVarShowAvoidanceDebug.GetValueOnGameThread())
			{
				DrawDebugString
				(
					worldPointer, 
					FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), k_debugVectorHeightAdjust),
					FString::Printf
					(
						TEXT("%d\nIsConvex: %d"),
						j,
						obstacles[i][j].m_isConvex
					),
					nullptr,
					FColor::Purple,
					0.1f,
					true,
					0.75f
				);
				DrawDebugLine
				(
					worldPointer,
					FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), k_debugVectorHeightAdjust),
					FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point + (obstacles[i][j].m_direction * 100.0f)), k_debugVectorHeightAdjust),
					FColor::Purple,
					false,
					0.1f,
					0u,
					k_debugVectorWidth
				);
				DrawDebugSphere
				(
					worldPointer,
					FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), k_debugVectorHeightAdjust),
					10.0f,
					4u,
					FColor::Purple,
					false,
					0.1f,
					0u,
					k_debugVectorWidth
				);
			}

			if (j == obstacles[i].Num() - 1)
			{
				break;
			}

			const FVector2D previousObstaclePointDir = j == 0 ? obstacles[i][0].m_direction : obstacles[i][j - 1].m_direction;
			CalculateORCALineForObstacleSegment(params, obstacles[i][j], obstacles[i][j + 1], previousObstaclePointDir, outORCALines);
		}
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

		const float effortCoefficient = GetEffortCoefficientForEntityPair(components, foundEntity);
		if (effortCoefficient == 0.0f)
		{
			continue;
		}

		CreateEntityORCALinesParamsPerEntity perEntityParams;
		perEntityParams.m_foundEntityLocation = ArgusMath::ToCartesianVector2(FVector2D(foundTransformComponent->m_transform.GetLocation()));
		perEntityParams.m_foundEntityVelocity = ArgusMath::ToCartesianVector2(FVector2D(foundTransformComponent->m_currentVelocity));
		perEntityParams.m_entityRadius = components.m_transformComponent->m_radius;

		FVector2D velocityToBoundaryOfVO = FVector2D::ZeroVector;
		ORCALine calculatedORCALine;
		FindORCALineAndVelocityToBoundaryPerEntity(params, perEntityParams, velocityToBoundaryOfVO, calculatedORCALine);

		calculatedORCALine.m_point = params.m_sourceEntityVelocity + (velocityToBoundaryOfVO * effortCoefficient);
		outORCALines.push_back(calculatedORCALine);
	}
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
		// No collision yet.
		const FVector2D cutoffCenterToRelativeVelocity = relativeVelocity - (params.m_inverseEntityPredictionTime * relativeLocation);
		const float cutoffCenterToRelativeVelocityLengthSqared = cutoffCenterToRelativeVelocity.SquaredLength();
		const float dotProduct = cutoffCenterToRelativeVelocity.Dot(relativeLocation);

		if (dotProduct > 0.0f && FMath::Square(dotProduct) > combinedRadiusSquared * cutoffCenterToRelativeVelocityLengthSqared)
		{
			const float cutoffCenterToRelativeVelocityLength = FMath::Sqrt(cutoffCenterToRelativeVelocityLengthSqared);
			const FVector2D unitCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity / cutoffCenterToRelativeVelocityLength;
			calculatedORCALine.m_direction = FVector2D(unitCutoffCenterToRelativeVelocity.Y, -unitCutoffCenterToRelativeVelocity.X);
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
	if (!navData)
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

	if (CVarShowAvoidanceDebug.GetValueOnGameThread())
	{
		const FVector zAdjust = FVector(0.0f, 0.0f, k_debugVectorHeightAdjust);
		DrawDebugCircle(worldPointer, entityLocation + zAdjust, ArgusECSConstants::k_avoidanceAgentSearchRadius, 20, FColor::Yellow, false, -1.0f, 0, k_debugVectorWidth, FVector::RightVector, FVector::ForwardVector, false);
		for (int32 i = 0; i < 4; ++i)
		{
			DrawDebugLine(worldPointer, queryShapePoints[i] + zAdjust, queryShapePoints[(i + 1) % 4] + zAdjust, FColor::Yellow, false, -1.0f, k_debugVectorWidth);
		}
	}

	TArray<FVector> unfilteredNavEdges;
	navData->FindOverlappingEdges(originLocation, TConstArrayView<FVector>(queryShapePoints), unfilteredNavEdges);

	int32 numNavEdges = unfilteredNavEdges.Num();
	outNavEdges.Reserve(numNavEdges);
	for (int32 i = 0; i < numNavEdges; i += 2)
	{
		const float searchRadiusSquared = FMath::Square(ArgusECSConstants::k_avoidanceAgentSearchRadius);
		const bool isFirstOutOfRange = (unfilteredNavEdges[i] - entityLocation).SquaredLength() > searchRadiusSquared;
		const bool isSecondOutOfRange = (unfilteredNavEdges[i + 1] - entityLocation).SquaredLength() > searchRadiusSquared;
		if (isFirstOutOfRange && isSecondOutOfRange)
		{
			continue;
		}

		outNavEdges.Add(unfilteredNavEdges[i]);
		outNavEdges.Add(unfilteredNavEdges[i + 1]);
	}
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
			return 1.0f;
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

void AvoidanceSystems::CalculateObstacles(const FVector2D& sourceEntityLocation, const TArray<FVector>& navEdges, TArray<TArray<ObstaclePoint>>& outObstacles)
{
	const int32 numNavEdges = navEdges.Num();
	if ((numNavEdges % 2) != 0 || numNavEdges == 0)
	{
		return;
	}

	for (int32 i = 0; i < numNavEdges; i += 2)
	{
		const FVector2D edgeVertex0 = ArgusMath::ToCartesianVector2(FVector2D(navEdges[i]));
		const FVector2D edgeVertex1 = ArgusMath::ToCartesianVector2(FVector2D(navEdges[i + 1]));

		bool handledEdge = false;
		for (int32 j = 0; j < outObstacles.Num(); ++j)
		{
			const int32 numObstaclesInChain = outObstacles[j].Num();
			if (numObstaclesInChain == 0)
			{
				continue;
			}

			const FVector2D startOfChainLocation = outObstacles[j][0].m_point;
			const FVector2D endOfChainLocation = outObstacles[j][numObstaclesInChain - 1].m_point;
			if (startOfChainLocation == edgeVertex0)
			{
				ObstaclePoint vertex1Obstacle;
				vertex1Obstacle.m_point = edgeVertex1;
				outObstacles[j].Insert(vertex1Obstacle, 0);
				handledEdge = true;
				break;
			}
			if (startOfChainLocation == edgeVertex1)
			{
				ObstaclePoint vertex0Obstacle;
				vertex0Obstacle.m_point = edgeVertex0;
				outObstacles[j].Insert(vertex0Obstacle, 0);
				handledEdge = true;
				break;
			}
			if (endOfChainLocation == edgeVertex0)
			{
				ObstaclePoint vertex1Obstacle;
				vertex1Obstacle.m_point = edgeVertex1;
				outObstacles[j].Add(vertex1Obstacle);
				handledEdge = true;
				break;
			}
			if (endOfChainLocation == edgeVertex1)
			{
				ObstaclePoint vertex0Obstacle;
				vertex0Obstacle.m_point = edgeVertex0;
				outObstacles[j].Add(vertex0Obstacle);
				handledEdge = true;
				break;
			}
		}

		if (handledEdge)
		{
			continue;
		}
		
		ObstaclePoint vertex0Obstacle, vertex1Obstacle;
		vertex0Obstacle.m_point = edgeVertex0;
		vertex1Obstacle.m_point = edgeVertex1;
		outObstacles.Add(TArray<ObstaclePoint>({ vertex0Obstacle, vertex1Obstacle }));
	}

	for (int32 i = 0; i < outObstacles.Num(); ++i)
	{
		CalculateDirectionAndConvexForObstacles(sourceEntityLocation, outObstacles[i]);
	}
}

void AvoidanceSystems::CalculateDirectionAndConvexForObstacles(const FVector2D& sourceEntityLocation, TArray<ObstaclePoint>& outObstacle)
{
	const int32 numObstaclePoints = outObstacle.Num();

	// Reverse the obstacle if it is ordered backwards relative to the entity we are calculating.
	if (FindAreaOfObstacleCartesian(outObstacle) > 0.0f)
	{
		const int32 halfObstaclePoints = numObstaclePoints / 2;
		for (int32 i = 0; i < halfObstaclePoints; ++i)
		{
			outObstacle.Swap(i, numObstaclePoints - (i + 1));
		}
	}

	for (int32 i = 0; i < numObstaclePoints; ++i)
	{
		const int32 nextIndex = (i + 1) % numObstaclePoints;
		outObstacle[i].m_direction = outObstacle[nextIndex].m_point - outObstacle[i].m_point;
		outObstacle[i].m_direction.Normalize();

		const int32 lastIndex = (i - 1) >= 0 ? (i - 1) : numObstaclePoints - 1;
		outObstacle[i].m_isConvex = ArgusMath::IsLeftOfCartesian(outObstacle[lastIndex].m_point, outObstacle[i].m_point, outObstacle[nextIndex].m_point);
	}
}

void AvoidanceSystems::CalculateORCALineForObstacleSegment(const CreateEntityORCALinesParams& params, ObstaclePoint obstaclePoint0, ObstaclePoint obstaclePoint1, const FVector2D& previousObstaclePointDir, std::vector<ORCALine>& outORCALines)
{
	const FVector2D relativeLocation0 = obstaclePoint0.m_point - params.m_sourceEntityLocation;
	const FVector2D relativeLocation1 = obstaclePoint1.m_point - params.m_sourceEntityLocation;

	// Check if the velocity obstacle of the obstacle is already covered by existing ORCA lines.
	for (int i = 0; i < outORCALines.size(); ++i)
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
			outORCALines.push_back(line);
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
			outORCALines.push_back(line);
			return;
		}
	}

	// Collision with obstacle segment
	if (sValue >= 0.0f && sValue <= 1.0f && squaredDistanceLine <= radiusSquared)
	{
		line.m_point = FVector2D::ZeroVector;
		line.m_direction = -obstaclePoint0.m_direction;
		outORCALines.push_back(line);
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
		outORCALines.push_back(line);
		return;
	}

	if (tValue > 1.0f && tRight < 0.0f)
	{
		// Project onto right cutoff circle
		const FVector2D unitW = (params.m_sourceEntityVelocity - rightCutoff).GetSafeNormal();
		line.m_direction = FVector2D(unitW.Y, -unitW.X);
		line.m_point = rightCutoff + (params.m_entityRadius * params.m_inverseObstaclePredictionTime * unitW);
		outORCALines.push_back(line);
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
		outORCALines.push_back(line);
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
		outORCALines.push_back(line);
		return;
	}

	if (isRightLegForeign)
	{
		return;
	}

	line.m_direction = -rightLegDirection;
	line.m_point = rightCutoff + (params.m_entityRadius * params.m_inverseObstaclePredictionTime * FVector2D(-line.m_direction.Y, line.m_direction.X));
	outORCALines.push_back(line);
}

void AvoidanceSystems::DrawORCADebugLines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const std::vector<ORCALine>& orcaLines, bool areObstacleLines, int startingLine)
{
	if (!CVarShowAvoidanceDebug.GetValueOnGameThread() || !worldPointer)
	{
		return;
	}

	const FVector relativeVelocityBasisTranslation = FVector(ArgusMath::ToUnrealVector2(params.m_sourceEntityLocation), 0.0f);
	const FQuat relativeVelocityBasisRotation = FRotationMatrix::MakeFromXZ(FVector::ForwardVector, FVector::UpVector).ToQuat();
	const FTransform basisTransform = FTransform(relativeVelocityBasisRotation, relativeVelocityBasisTranslation);

	FColor debugColor = areObstacleLines ? FColor::Purple : FColor::Cyan;

	for (int i = startingLine; i < orcaLines.size(); ++i)
	{
		const FVector worldspacePoint = basisTransform.TransformPosition(FVector(ArgusMath::ToUnrealVector2(orcaLines[i].m_point), 0.0f));
		const FVector worldspaceDirection = basisTransform.TransformVector(FVector(ArgusMath::ToUnrealVector2(orcaLines[i].m_direction), 0.0f));
		const FVector worldspaceOrthogonalDirectionScaled = worldspaceDirection.Cross(FVector::UpVector) * 1000.0f;

		DrawDebugSphere(worldPointer, worldspacePoint, 10.0f, 10u, debugColor, false, -1.0f, 0u, k_debugVectorWidth);
		DrawDebugLine(worldPointer, worldspacePoint, worldspacePoint + (worldspaceDirection * 100.0f), debugColor, false, -1.0f, 0u, k_debugVectorWidth);
		DrawDebugLine(worldPointer, worldspacePoint - worldspaceOrthogonalDirectionScaled, worldspacePoint + worldspaceOrthogonalDirectionScaled, debugColor, false, -1.0f, 0u, k_debugVectorWidth);
	}
}

#pragma endregion
