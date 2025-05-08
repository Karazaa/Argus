// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePoint.h"
#include "ArgusECSConstants.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "DrawDebugHelpers.h"

void ObstaclePoint::DrawDebugObstaclePoint(UWorld* worldPointer) const
{
	DrawDebugString
	(
		worldPointer,
		FVector(ArgusMath::ToUnrealVector2(m_point), ArgusECSConstants::k_debugDrawHeightAdjustment),
		FString::Printf
		(
			TEXT("IsConvex: %s"),
			m_isConvex ? TEXT("true") : TEXT("false")
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
		FVector(ArgusMath::ToUnrealVector2(m_point), ArgusECSConstants::k_debugDrawHeightAdjustment),
		FVector(ArgusMath::ToUnrealVector2(m_point + (m_direction * 100.0f)), ArgusECSConstants::k_debugDrawHeightAdjustment),
		FColor::Purple,
		false,
		0.1f,
		0u,
		ArgusECSConstants::k_debugDrawLineWidth
	);
	DrawDebugSphere
	(
		worldPointer,
		FVector(ArgusMath::ToUnrealVector2(m_point), ArgusECSConstants::k_debugDrawHeightAdjustment),
		10.0f,
		4u,
		FColor::Purple,
		false,
		0.1f,
		0u,
		ArgusECSConstants::k_debugDrawLineWidth
	);
}

const ObstaclePoint& ObstaclePointArray::GetHead() const
{
	if (Num() == 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access index of empty %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}
	
	return (GetData()[0]);
}

const ObstaclePoint& ObstaclePointArray::GetTail() const
{
	if (Num() == 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access index of empty %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (GetData()[Num() - 1]);
}

const ObstaclePoint& ObstaclePointArray::GetPrevious(int32 index) const
{
	if (index >= Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (GetData()[(index - 1 + Num()) % Num()]);
}

const ObstaclePoint& ObstaclePointArray::GetNext(int32 index) const
{
	if (index >= Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (GetData()[(index + 1) % Num()]);
}

void ObstaclePointArray::FillInBetweenObstaclePoints(const ObstaclePoint& fromPoint, const ObstaclePoint& toPoint, TArray<ObstaclePoint>& outPoints)
{
	const FVector2D betweenObstaclePoints = toPoint.m_point - fromPoint.m_point;
	float distanceBetweenObstaclePoints = betweenObstaclePoints.Length();
	const FVector2D directionBetweenObstaclePoints = betweenObstaclePoints / distanceBetweenObstaclePoints;

	outPoints.Reserve(2 + FMath::FloorToInt32(distanceBetweenObstaclePoints / ArgusECSConstants::k_avoidanceObstacleSplitDistance));

	float iterations = 0.0f;
	while (distanceBetweenObstaclePoints > ArgusECSConstants::k_avoidanceObstacleSplitDistance)
	{
		iterations += 1.0f;
		distanceBetweenObstaclePoints -= ArgusECSConstants::k_avoidanceObstacleSplitDistance;

		ObstaclePoint& pointToInsert = outPoints.Emplace_GetRef();
		pointToInsert.m_point = fromPoint.m_point + ((iterations * ArgusECSConstants::k_avoidanceObstacleSplitDistance) * directionBetweenObstaclePoints);
	}
}

void ObstaclePointArray::AddObstaclePointsWithFillIn(const ObstaclePoint& instigatingObstacle, bool addToHead)
{
	TArray<ObstaclePoint> obstaclePointsToAdd;

	const ObstaclePoint& attachPoint = addToHead ? GetHead() : GetTail();
	const ObstaclePoint& startPoint = addToHead ? instigatingObstacle : attachPoint;
	const ObstaclePoint& endPoint = addToHead ? attachPoint : instigatingObstacle;
	const int32 index = addToHead ? 0 : Num();

	if (addToHead)
	{
		obstaclePointsToAdd.Add(startPoint);
	}

	FillInBetweenObstaclePoints(startPoint, endPoint, obstaclePointsToAdd);

	if (!addToHead)
	{
		obstaclePointsToAdd.Add(endPoint);
	}

	Insert(obstaclePointsToAdd, index);
}

void ObstaclePointArray::Reverse()
{
	const int32 halfObstaclePoints = Num() / 2;
	for (int32 i = 0; i < halfObstaclePoints; ++i)
	{
		Swap(i, Num() - (i + 1));
	}
}

void ObstaclePointArray::AppendOtherToThis(ObstaclePointArray& other)
{
	if (other.Num() < 2)
	{
		return;
	}

	TArray<ObstaclePoint> obstaclePointsToAdd;
	FillInBetweenObstaclePoints(GetTail(), other[1], obstaclePointsToAdd);
	if (obstaclePointsToAdd.Num() > 0)
	{
		Insert(obstaclePointsToAdd, Num());
	}

	Append(&other[1], other.Num() - 1);
}

void ObstaclePointArray::CloseLoop()
{
	if (GetHead().m_point == GetTail().m_point)
	{
		RemoveAt(Num() - 1, EAllowShrinking::No);
	}

	TArray<ObstaclePoint> obstaclePointsToAdd;
	FillInBetweenObstaclePoints(GetTail(), GetHead(), obstaclePointsToAdd);
	if (obstaclePointsToAdd.Num() > 0)
	{
		Insert(obstaclePointsToAdd, Num());
	}
}