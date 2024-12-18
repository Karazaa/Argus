// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
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
		FVector2D m_sourceEntityLocation = FVector2D::ZeroVector;
		FVector2D m_sourceEntityVelocity = FVector2D::ZeroVector;
		float m_deltaTime = 0.0f;
		float m_inversePredictionTime = 0.0f;
	};
	struct CreateEntityORCALinesParamsPerEntity
	{
		FVector2D m_foundEntityLocation = FVector2D::ZeroVector;
		FVector2D m_foundEntityVelocity = FVector2D::ZeroVector;
		float m_entityRadius = 45.0f;
	};

	static void CreateObstacleORCALines(UWorld* worldPointer, const TransformSystems::TransformSystemsComponentArgs& components, std::vector<ORCALine>& outORCALines);
	static void CreateEntityORCALines(const CreateEntityORCALinesParams& params, const TransformSystems::TransformSystemsComponentArgs& components, std::vector<uint16>& foundEntityIds, std::vector<ORCALine>& outORCALines);
	static void FindORCALineAndVelocityToBoundaryPerEntity(const CreateEntityORCALinesParams& params, const CreateEntityORCALinesParamsPerEntity& perEntityParams, FVector2D& velocityToBoundaryOfVO, ORCALine& orcaLine);
	static bool OneDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int lineIndex, FVector2D& resultingVelocity);
	static bool TwoDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int& failureLine);
	static void ThreeDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const int lineIndex, const int numStaticObstacleORCALines, FVector2D& resultingVelocity);
	
	static void RetrieveRelevantNavEdges(UWorld* worldPointer, const TransformSystems::TransformSystemsComponentArgs& components, TArray<FVector>& outNavEdges);
	static float GetEffortCoefficientForEntityPair(const TransformSystems::TransformSystemsComponentArgs& sourceEntityComponents, const ArgusEntity& foundEntity);
};