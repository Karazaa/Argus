// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ObstaclePoint.h"
#include "SystemArgumentDefinitions/TransformSystemsArgs.h"

class AvoidanceSystems
{
public:
	static void RunSystems(UWorld* worldPointer, float deltaTime);

	static void					ProcessORCAvoidance(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components, const NearbyEntitiesComponent* nearbyEntitiesComponent);
	static ArgusEntity			GetAvoidanceGroupLeader(const ArgusEntity& entity);
	static bool					AreInSameAvoidanceGroup(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static void					DecrementIdleEntitiesInGroup(const ArgusEntity& entity);
	static TOptional<FVector>	GetAvoidanceGroupDestinationLocation(const TransformSystemsArgs& components);
	static TOptional<FVector>	GetAvoidanceGroupSourceLocation(const TransformSystemsArgs& components);
	static FVector2D			GetFlockingVelocity(const TransformSystemsArgs& components);

private:
	struct ORCALine
	{
		FVector2D m_direction = FVector2D::ZeroVector;
		FVector2D m_point = FVector2D::ZeroVector;
	};
	struct CreateEntityORCALinesParams
	{
		FTransform m_basisTransform = FTransform::Identity;
		FVector m_sourceEntityLocation3D = FVector::ZeroVector;
		FVector2D m_sourceEntityLocation = FVector2D::ZeroVector;
		FVector2D m_sourceEntityVelocity = FVector2D::ZeroVector;
		float m_deltaTime = 0.0f;
		float m_defaultInverseEntityPredictionTime = 0.0f;
		float m_inverseObstaclePredictionTime = 0.0f;
		float m_entityRadius = 45.0f;
		const SpatialPartitioningComponent* m_spatialPartitioningComponent = nullptr;
	};
	struct CreateEntityORCALinesParamsPerEntity
	{
		FVector2D m_foundEntityLocation = FVector2D::ZeroVector;
		FVector2D m_foundEntityVelocity = FVector2D::ZeroVector;
		float m_entityRadius = 45.0f;
		float m_inverseEntityPredictionTime = 0.0f;
	};

	static void			CreateObstacleORCALines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TransformSystemsArgs& components, TArray<ORCALine>& outORCALines);
	static void			CreateEntityORCALines(const CreateEntityORCALinesParams& params, const TransformSystemsArgs& components, const TArray<uint16>& foundEntityIds, TArray<ORCALine>& outORCALines, FVector2D& outDesiredVelocity);
	static void			FindORCALineAndVelocityToBoundaryPerEntity(const CreateEntityORCALinesParams& params, const CreateEntityORCALinesParamsPerEntity& perEntityParams, FVector2D& velocityToBoundaryOfVO, ORCALine& orcaLine);
	static bool			OneDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int32 lineIndex, FVector2D& resultingVelocity);
	static bool			TwoDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int32& failureLine);
	static void			ThreeDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const int32 lineIndex, const int numStaticObstacleORCALines, FVector2D& resultingVelocity);
	static FVector2D	GetDesiredVelocity(const TransformSystemsArgs& components);

	static float		GetEffortCoefficientForEntityPair(const TransformSystemsArgs& sourceEntityComponents, const ArgusEntity& foundEntity);
	static float		GetEffortCoefficientForAvoidanceGroupPair(const TransformSystemsArgs& sourceEntityComponents, const ArgusEntity& foundEntity, const AvoidanceGroupingComponent* sourceGroupComponent, const AvoidanceGroupingComponent* foundGroupComponent);
	static bool			ShouldReturnCombatEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, float& coefficient);
	static bool			ShouldReturnConstructionEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, float& coefficient);
	static bool			ShouldReturnCarrierEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const ArgusEntity& foundEntity, const TaskComponent* foundEntityTaskComponent, float& coefficient);
	static bool			ShouldReturnAvoidancePriorityEffortCoefficient(const AvoidanceGroupingComponent* sourceEntityAvoidanceGroupingComponent, const AvoidanceGroupingComponent* foundEntityAvoidanceGroupingComponent, float& coefficient);
	static float		FindAreaOfObstacleCartesian(const TArray<ObstaclePoint>& obstaclePoints);
	
	static void			CalculateORCALineForObstacleSegment(const CreateEntityORCALinesParams& params, ObstaclePoint obstaclePoint0, ObstaclePoint obstaclePoint1, const FVector2D& previousObstaclePointDir, TArray<ORCALine>& outORCALines);
	static void			DrawORCADebugLines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TArray<ORCALine>& orcaLines, bool areObstacleLines, int32 startingLine);
};