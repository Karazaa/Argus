// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ComponentDependencies/ObstaclePoint.h"
#include "TransformSystems.h"


class AvoidanceSystems
{
public:
	static void RunSystems(UWorld* worldPointer, float deltaTime);
	static void ProcessORCAvoidance(UWorld* worldPointer, float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components);
	
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
		float m_inverseEntityPredictionTime = 0.0f;
		float m_inverseObstaclePredictionTime = 0.0f;
		float m_entityRadius = 45.0f;
		const SpatialPartitioningComponent* m_spatialPartitioningComponent = nullptr;
	};
	struct CreateEntityORCALinesParamsPerEntity
	{
		FVector2D m_foundEntityLocation = FVector2D::ZeroVector;
		FVector2D m_foundEntityVelocity = FVector2D::ZeroVector;
		float m_entityRadius = 45.0f;
	};

	static void			CreateObstacleORCALines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, TArray<ORCALine>& outORCALines);
	static void			CreateEntityORCALines(const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, TArray<uint16>& foundEntityIds, TArray<ORCALine>& outORCALines);
	static void			FindORCALineAndVelocityToBoundaryPerEntity(const CreateEntityORCALinesParams& params, const CreateEntityORCALinesParamsPerEntity& perEntityParams, FVector2D& velocityToBoundaryOfVO, ORCALine& orcaLine);
	static bool			OneDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int32 lineIndex, FVector2D& resultingVelocity);
	static bool			TwoDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int32& failureLine);
	static void			ThreeDimensionalLinearProgram(const TArray<ORCALine>& orcaLines, const float radius, const int32 lineIndex, const int numStaticObstacleORCALines, FVector2D& resultingVelocity);
	
	static float		GetEffortCoefficientForEntityPair(const TransformSystems::TransformSystemsComponentArgs& sourceEntityComponents, const ArgusEntity& foundEntity);
	static float		FindAreaOfObstacleCartesian(const TArray<ObstaclePoint>& obstaclePoints);
	
	static void			CalculateORCALineForObstacleSegment(const CreateEntityORCALinesParams& params, ObstaclePoint obstaclePoint0, ObstaclePoint obstaclePoint1, const FVector2D& previousObstaclePointDir, TArray<ORCALine>& outORCALines);
	static void			DrawORCADebugLines(UWorld* worldPointer, const CreateEntityORCALinesParams& params, const TArray<ORCALine>& orcaLines, bool areObstacleLines, int32 startingLine);
};