// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePoint.h"
#include "ArgusECSConstants.h"
#include "ArgusLogging.h"
#include "ArgusMath.h"
#include "DynamicAllocComponentDefinitions/GlobalSettingsComponent.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"

void FObstaclePoint::DrawDebugObstaclePoint(UWorld* worldPointer, float duration, bool shouldShowText, bool isPointElevated) const
{
	const FColor color = isPointElevated ? FColor::Magenta : FColor::Purple;
	if (shouldShowText)
	{
		DrawDebugString
		(
			worldPointer,
			FVector(ArgusMath::ToUnrealVector2(m_point), ArgusECSConstants::k_debugDrawHeightAdjustment + m_height),
			FString::Printf
			(
				TEXT("IsConvex: %s"),
				m_isConvex ? TEXT("true") : TEXT("false")
			),
			nullptr,
			color,
			duration,
			true,
			0.75f
		);
	}
	DrawDebugLine
	(
		worldPointer,
		FVector(ArgusMath::ToUnrealVector2(m_point), ArgusECSConstants::k_debugDrawHeightAdjustment + m_height),
		FVector(ArgusMath::ToUnrealVector2(m_point + (m_direction * 100.0f)), ArgusECSConstants::k_debugDrawHeightAdjustment + m_height),
		color,
		false,
		duration,
		0u,
		ArgusECSConstants::k_debugDrawLineWidth
	);
	if (m_isAlias)
	{
		DrawDebugBox
		(
			worldPointer,
			FVector(ArgusMath::ToUnrealVector2(m_point), ArgusECSConstants::k_debugDrawHeightAdjustment + m_height),
			FVector(10.0f, 10.0f, 10.0f),
			FQuat::Identity,
			color,
			false,
			duration,
			0u,
			ArgusECSConstants::k_debugDrawLineWidth
		);
	}
	else
	{
		DrawDebugSphere
		(
			worldPointer,
			FVector(ArgusMath::ToUnrealVector2(m_point), ArgusECSConstants::k_debugDrawHeightAdjustment + m_height),
			10.0f,
			4u,
			color,
			false,
			duration,
			0u,
			ArgusECSConstants::k_debugDrawLineWidth
		);
	}
}
#endif //#!UE_BUILD_SHIPPING

const FObstaclePoint& FObstaclePointArray::GetHead() const
{
	if (m_obstaclePoints.Num() == 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access index of empty %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}
	
	return (m_obstaclePoints[0]);
}

const FObstaclePoint& FObstaclePointArray::GetTail() const
{
	if (m_obstaclePoints.Num() == 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access index of empty %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (m_obstaclePoints[m_obstaclePoints.Num() - 1]);
}

const int32 FObstaclePointArray::GetPreviousIndex(int32 index) const
{
	if (index >= m_obstaclePoints.Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (index - 1 + m_obstaclePoints.Num()) % m_obstaclePoints.Num();
}

const FObstaclePoint& FObstaclePointArray::GetPrevious(int32 index) const
{
	return (m_obstaclePoints[GetPreviousIndex(index)]);
}

const int32 FObstaclePointArray::GetPreviousNonAliasIndex(int32 index) const
{
	int32 iterations = 0;
	index = GetPreviousIndex(index);
	const FObstaclePoint* newPoint = &m_obstaclePoints[index];

	while (newPoint->m_isAlias && iterations < m_obstaclePoints.Num())
	{
		index = GetPreviousIndex(index);
		newPoint = &m_obstaclePoints[index];
		iterations++;
	}

	return index;
}

const int32 FObstaclePointArray::GetCurrentNonAliasIndex(int32 index) const
{
	if (index >= m_obstaclePoints.Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	if (m_obstaclePoints[index].m_isAlias)
	{
		return GetPreviousNonAliasIndex(index);
	}

	return index;
}

const FObstaclePoint& FObstaclePointArray::GetPreviousNonAlias(int32 index) const
{
	return m_obstaclePoints[GetPreviousNonAliasIndex(index)];
}

const int32 FObstaclePointArray::GetNextIndex(int32 index) const
{
	if (index >= m_obstaclePoints.Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (index + 1) % m_obstaclePoints.Num();
}

const FObstaclePoint& FObstaclePointArray::GetNext(int32 index) const
{
	return (m_obstaclePoints[GetNextIndex(index)]);
}

const int32 FObstaclePointArray::GetNextNonAliasIndex(int32 index) const
{
	int32 iterations = 0;
	index = GetNextIndex(index);
	const FObstaclePoint* newPoint = &m_obstaclePoints[index];

	while (newPoint->m_isAlias && iterations < m_obstaclePoints.Num())
	{
		index = GetNextIndex(index);
		newPoint = &m_obstaclePoints[index];
		iterations++;
	}

	return index;
}

const FObstaclePoint& FObstaclePointArray::GetNextNonAlias(int32 index) const
{
	return m_obstaclePoints[GetNextNonAliasIndex(index)];
}

void FObstaclePointArray::FillInBetweenObstaclePoints(const FObstaclePoint& fromPoint, const FObstaclePoint& toPoint, TArray<FObstaclePoint>& outPoints)
{
	const GlobalSettingsComponent* settings = GlobalSettingsComponent::Get();
	ARGUS_RETURN_ON_NULL(settings, ArgusECSLog);

	const float deltaHeight = toPoint.m_height - fromPoint.m_height;
	const FVector2D betweenObstaclePoints = toPoint.m_point - fromPoint.m_point;
	const float distanceBetweenObstaclePoints = betweenObstaclePoints.Length();
	if (distanceBetweenObstaclePoints < settings->m_maxObstaclePointDistance)
	{
		return;
	}

	const FVector2D directionBetweenObstaclePoints = ArgusMath::SafeDivide(betweenObstaclePoints, distanceBetweenObstaclePoints);
	const float slopeBetweenObstaclePoints = ArgusMath::SafeDivide(deltaHeight, distanceBetweenObstaclePoints);

	const int32 numAddedPoints = FMath::FloorToInt32(ArgusMath::SafeDivide(distanceBetweenObstaclePoints, settings->m_maxObstaclePointDistance));
	const float distanceIncrement = ArgusMath::SafeDivide(distanceBetweenObstaclePoints, (numAddedPoints + 1));
	outPoints.Reserve(2 + numAddedPoints);

	for (int32 i = 1; i <= numAddedPoints; ++i)
	{
		const float distanceThisIteration = static_cast<float>(i) * distanceIncrement;
		FObstaclePoint& pointToInsert = outPoints.Emplace_GetRef();
		pointToInsert.m_point = fromPoint.m_point + (distanceThisIteration * directionBetweenObstaclePoints);
		pointToInsert.m_height = fromPoint.m_height + (distanceThisIteration * slopeBetweenObstaclePoints);
		pointToInsert.m_isAlias = true;
	}
}

void FObstaclePointArray::AddObstaclePointsWithFillIn(const FObstaclePoint& instigatingObstacle, bool addToHead)
{
	TArray<FObstaclePoint> obstaclePointsToAdd;

	const FObstaclePoint& attachPoint = addToHead ? GetHead() : GetTail();
	const FObstaclePoint& startPoint = addToHead ? instigatingObstacle : attachPoint;
	const FObstaclePoint& endPoint = addToHead ? attachPoint : instigatingObstacle;
	const int32 index = addToHead ? 0 : m_obstaclePoints.Num();

	if (addToHead)
	{
		obstaclePointsToAdd.Add(startPoint);
	}

	FillInBetweenObstaclePoints(startPoint, endPoint, obstaclePointsToAdd);

	if (!addToHead)
	{
		obstaclePointsToAdd.Add(endPoint);
	}

	m_obstaclePoints.Insert(obstaclePointsToAdd, index);
}

void FObstaclePointArray::Reverse()
{
	const int32 halfObstaclePoints = m_obstaclePoints.Num() / 2;
	for (int32 i = 0; i < halfObstaclePoints; ++i)
	{
		m_obstaclePoints.Swap(i, m_obstaclePoints.Num() - (i + 1));
	}
}

void FObstaclePointArray::AppendOtherToThis(FObstaclePointArray& other)
{
	if (other.m_obstaclePoints.Num() < 2)
	{
		return;
	}

	TArray<FObstaclePoint> obstaclePointsToAdd;
	FillInBetweenObstaclePoints(GetTail(), other.m_obstaclePoints[1], obstaclePointsToAdd);
	if (obstaclePointsToAdd.Num() > 0)
	{
		m_obstaclePoints.Insert(obstaclePointsToAdd, m_obstaclePoints.Num());
	}

	m_obstaclePoints.Append(&other.m_obstaclePoints[1], other.m_obstaclePoints.Num() - 1);
}

void FObstaclePointArray::CloseLoop()
{
	if (GetHead().m_point == GetTail().m_point)
	{
		m_obstaclePoints.RemoveAt(m_obstaclePoints.Num() - 1, EAllowShrinking::No);
	}

	TArray<FObstaclePoint> obstaclePointsToAdd;
	FillInBetweenObstaclePoints(GetTail(), GetHead(), obstaclePointsToAdd);
	if (obstaclePointsToAdd.Num() > 0)
	{
		m_obstaclePoints.Insert(obstaclePointsToAdd, m_obstaclePoints.Num());
	}
}

void FObstaclePointArray::ConsolidateNearbyPoints()
{
	const GlobalSettingsComponent* settings = GlobalSettingsComponent::Get();
	ARGUS_RETURN_ON_NULL(settings, ArgusECSLog);

	const float thresholdSquared = FMath::Square(settings->m_minObstaclePointDistance);
	TArray<FObstaclePoint> finalObstaclePoints;
	finalObstaclePoints.Reserve(m_obstaclePoints.Num());
	finalObstaclePoints.Add(GetTail());

	int32 j = 0;
	for (int32 i = 0; i < m_obstaclePoints.Num() - 1; ++i)
	{
		const float distSquared = FVector2D::DistSquared(finalObstaclePoints[j].m_point, m_obstaclePoints[i].m_point);
		if (distSquared < thresholdSquared)
		{
			finalObstaclePoints[j].m_point = ArgusMath::Average(finalObstaclePoints[j].m_point, m_obstaclePoints[i].m_point);
			finalObstaclePoints[j].m_isAlias = false;
		}
		else
		{
			finalObstaclePoints.Add(m_obstaclePoints[i]);
			++j;
		}
	}

	m_obstaclePoints.Reset();

	for (int32 i = 0; i < finalObstaclePoints.Num(); ++i)
	{
		m_obstaclePoints.Add(finalObstaclePoints[i]);
	}
}

bool FObstaclePointArray::IsPointElevated(int32 index) const
{
	if (index < 0 || index >= m_obstaclePoints.Num())
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Index, %d, was out of range of the %s"), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		return false;
	}

	return (m_obstaclePoints[index].m_height > m_floorHeight);
}

bool FObstaclePointArray::IsNextPointElevated(int32 index) const
{
	if (index < 0 || index >= m_obstaclePoints.Num())
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Index, %d, was out of range of the %s"), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		return false;
	}

	return GetNext(index).m_height > m_floorHeight;
}