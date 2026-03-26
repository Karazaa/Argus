// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ObstaclePoint.h"
#include "SystemArgumentDefinitions/TransformSystemsArgs.h"

class AvoidanceSystems
{
public:
	static void RunSystems(UWorld* worldPointer, float deltaTime);

	static void					ProcessORCAvoidance(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components, const NearbyEntitiesComponent* nearbyEntitiesComponent, float inverseEntityPredictionTime = 1.0f, float inverseObstaclePredictionTime = 1.0f);
	static ArgusEntity			GetAvoidanceGroupLeader(ArgusEntity entity);
	static bool					AreInSameAvoidanceGroup(ArgusEntity entity, ArgusEntity otherEntity);
	static void					DecrementIdleEntitiesInGroup(ArgusEntity entity);
	static TOptional<FVector>	GetAvoidanceGroupDestinationLocation(const TransformSystemsArgs& components);
	static TOptional<FVector>	GetAvoidanceGroupSourceLocation(const TransformSystemsArgs& components);
	static FVector2D			GetFlockingVelocity(const TransformSystemsArgs& components);
	static float				GetEntityAvoidanceRange(ArgusEntity entity);
	static float				GetObstacleAvoidanceRange(ArgusEntity entity);

	template<typename Function>
	static void IterateEntitiesInAvoidanceGroup(ArgusEntity entity, Function&& function)
	{
		ARGUS_RETURN_ON_INVALID_ENTITY(entity, ArgusECSLog);

		ArgusEntity groupLeader = GetAvoidanceGroupLeader(entity);
		if (!groupLeader)
		{
			function(entity);
			return;
		}

		const AvoidanceGroupingComponent* groupLeaderComponent = groupLeader.GetComponent<AvoidanceGroupingComponent>();
		ARGUS_RETURN_ON_NULL(groupLeaderComponent, ArgusECSLog);

		for (int32 i = 0; i < groupLeaderComponent->m_entityIdsInGroup.Num(); ++i)
		{
			ArgusEntity memberEntity = ArgusEntity::RetrieveEntity(groupLeaderComponent->m_entityIdsInGroup[i]);
			if (!memberEntity)
			{
				continue;
			}

			function(memberEntity);
		}
	}

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
		float m_adjacentEntityRange = 150.0f;
		float m_adjacentObstacleRange = 150.0f;
		SpatialPartitioningComponent* m_spatialPartitioningComponent = nullptr;
	};
	struct CreateEntityORCALinesParamsPerEntity
	{
		FVector2D m_foundEntityLocation = FVector2D::ZeroVector;
		FVector2D m_foundEntityVelocity = FVector2D::ZeroVector;
		float m_entityRadius = 45.0f;
		float m_inverseEntityPredictionTime = 0.0f;
		float m_sameAvoidanceGroupDistanceThreshold = 0.0f;
		bool  m_inSameAvoidanceGroup = false;
	};

	static void			CreateObstacleORCALines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TransformSystemsArgs& components, const TArray<ObstacleIndicies>& obstacleIndicies, TArray<ORCALine>& outORCALines);
	static void			CreateEntityORCALines(const CreateEntityORCALinesParams& params, const TransformSystemsArgs& components, const NearbyEntitiesComponent* nearbyEntitiesComponent, TArray<ORCALine>& outORCALines, FVector2D& outDesiredVelocity);
	static bool			FindORCALineAndVelocityToBoundaryPerEntity(const CreateEntityORCALinesParams& params, const CreateEntityORCALinesParamsPerEntity& perEntityParams, FVector2D& velocityToBoundaryOfVO, ORCALine& orcaLine);
	static bool			OneDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int32 lineIndex, FVector2D& resultingVelocity);
	static bool			TwoDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int32& failureLine);
	static void			ThreeDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const int32 lineIndex, const int numStaticObstacleORCALines, FVector2D& resultingVelocity);
	static FVector2D	GetDesiredVelocity(const TransformSystemsArgs& components, bool isInRangeOfObstacles);
	static FVector		GetDesiredDirection(const TransformSystemsArgs& components, bool isInRangeOfObstacles);

	static float		GetEffortCoefficientForEntityPair(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const AvoidanceGroupingComponent* sourceGroupingComponent, const AvoidanceGroupingComponent* foundGroupingComponent, bool inSameAvoidanceGroup);
	static float		GetEffortCoefficientForAvoidanceGroupPair(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const AvoidanceGroupingComponent* sourceGroupComponent, const AvoidanceGroupingComponent* foundGroupComponent);
	static bool			ShouldReturnMovabilityEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, float& coefficient);
	static bool			ShouldReturnCombatEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, bool inSameAvoidanceGroup, float& coefficient);
	static bool			ShouldReturnConstructionEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, float& coefficient);
	static bool			ShouldReturnCarrierEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const TaskComponent* foundEntityTaskComponent, float& coefficient);
	static bool			ShouldReturnTargetEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, bool inSameAvoidanceGroup, float& coefficient);
	static bool			ShouldReturnAvoidancePriorityEffortCoefficient(const AvoidanceGroupingComponent* sourceEntityAvoidanceGroupingComponent, const AvoidanceGroupingComponent* foundEntityAvoidanceGroupingComponent, float& coefficient);
	static bool			ShouldReturnMovementTaskEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, const TaskComponent* foundEntityTaskComponent, bool inSameAvoidanceGroup, float& coefficient);
	static bool			ShouldReturnResourceExtractionEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, const TaskComponent* foundEntityTaskComponent, bool inSameAvoidanceGroup, float& coefficient);
	static bool			ShouldReturnStaticFlockingEffortCoefficient(const TransformSystemsArgs& sourceEntityComponents, ArgusEntity foundEntity, float& coefficient);
	static float		FindAreaOfObstacleCartesian(const TArray<ObstaclePoint>& obstaclePoints);
	
	static void			CalculateORCALineForObstacleSegment(const CreateEntityORCALinesParams& params, ObstaclePoint obstaclePoint0, ObstaclePoint obstaclePoint1, const FVector2D& previousObstaclePointDir, TArray<ORCALine>& outORCALines);
	
	static void			PopulateAvoidanceRanges(ArgusEntity entity, float& outEntityRange, float& outObstacleRange);
	
#if !UE_BUILD_SHIPPING
	static void			DrawORCADebugLines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TArray<ORCALine>& orcaLines, bool areObstacleLines, int32 startingLine);
#endif //!UE_BUILD_SHIPPING
};