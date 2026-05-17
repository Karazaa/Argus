// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePoint.h"
#include "ArgusECSConstants.h"
#include "ArgusLogging.h"
#include "ArgusMath.h"
#include "DynamicAllocComponentDefinitions/GlobalSettingsComponent.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"

void ObstaclePoint::DrawDebugObstaclePoint(UWorld* worldPointer, float duration, bool shouldShowText, bool isPointElevated) const
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

const int32 ObstaclePointArray::GetPreviousIndex(int32 index) const
{
	if (index >= Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (index - 1 + Num()) % Num();
}

const ObstaclePoint& ObstaclePointArray::GetPrevious(int32 index) const
{
	return (GetData()[GetPreviousIndex(index)]);
}

const int32 ObstaclePointArray::GetPreviousNonAliasIndex(int32 index) const
{
	int32 iterations = 0;
	index = GetPreviousIndex(index);
	const ObstaclePoint* newPoint = &GetData()[index];

	while (newPoint->m_isAlias && iterations < Num())
	{
		index = GetPreviousIndex(index);
		newPoint = &GetData()[index];
		iterations++;
	}

	return index;
}

const int32 ObstaclePointArray::GetCurrentNonAliasIndex(int32 index) const
{
	if (index >= Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	if (GetData()[index].m_isAlias)
	{
		return GetPreviousNonAliasIndex(index);
	}

	return index;
}

const ObstaclePoint& ObstaclePointArray::GetPreviousNonAlias(int32 index) const
{
	return GetData()[GetPreviousNonAliasIndex(index)];
}

const int32 ObstaclePointArray::GetNextIndex(int32 index) const
{
	if (index >= Num() || index < 0)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to access invalid index(%d) of %s."), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		verify(false);
	}

	return (index + 1) % Num();
}

const ObstaclePoint& ObstaclePointArray::GetNext(int32 index) const
{
	return (GetData()[GetNextIndex(index)]);
}

const int32 ObstaclePointArray::GetNextNonAliasIndex(int32 index) const
{
	int32 iterations = 0;
	index = GetNextIndex(index);
	const ObstaclePoint* newPoint = &GetData()[index];

	while (newPoint->m_isAlias && iterations < Num())
	{
		index = GetNextIndex(index);
		newPoint = &GetData()[index];
		iterations++;
	}

	return index;
}

const ObstaclePoint& ObstaclePointArray::GetNextNonAlias(int32 index) const
{
	return GetData()[GetNextNonAliasIndex(index)];
}

void ObstaclePointArray::FillInBetweenObstaclePoints(const ObstaclePoint& fromPoint, const ObstaclePoint& toPoint, TArray<ObstaclePoint>& outPoints)
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
		ObstaclePoint& pointToInsert = outPoints.Emplace_GetRef();
		pointToInsert.m_point = fromPoint.m_point + (distanceThisIteration * directionBetweenObstaclePoints);
		pointToInsert.m_height = fromPoint.m_height + (distanceThisIteration * slopeBetweenObstaclePoints);
		pointToInsert.m_isAlias = true;
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

void ObstaclePointArray::ConsolidateNearbyPoints()
{
	const GlobalSettingsComponent* settings = GlobalSettingsComponent::Get();
	ARGUS_RETURN_ON_NULL(settings, ArgusECSLog);

	const float thresholdSquared = FMath::Square(settings->m_minObstaclePointDistance);
	TArray<ObstaclePoint> finalObstaclePoints;
	finalObstaclePoints.Reserve(Num());
	finalObstaclePoints.Add(GetTail());

	int32 j = 0;
	for (int32 i = 0; i < Num() - 1; ++i)
	{
		const float distSquared = FVector2D::DistSquared(finalObstaclePoints[j].m_point, GetData()[i].m_point);
		if (distSquared < thresholdSquared)
		{
			finalObstaclePoints[j].m_point = ArgusMath::Average(finalObstaclePoints[j].m_point, GetData()[i].m_point);
			finalObstaclePoints[j].m_isAlias = false;
		}
		else
		{
			finalObstaclePoints.Add(GetData()[i]);
			++j;
		}
	}

	Reset();

	for (int32 i = 0; i < finalObstaclePoints.Num(); ++i)
	{
		Add(finalObstaclePoints[i]);
	}
}

bool ObstaclePointArray::IsPointElevated(int32 index) const
{
	if (index < 0 || index >= Num())
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Index, %d, was out of range of the %s"), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		return false;
	}

	return (GetData()[index].m_height > m_floorHeight);
}

bool ObstaclePointArray::IsNextPointElevated(int32 index) const
{
	if (index < 0 || index >= Num())
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Index, %d, was out of range of the %s"), ARGUS_FUNCNAME, index, ARGUS_NAMEOF(ObstaclePointArray));
		return false;
	}

	return GetNext(index).m_height > m_floorHeight;
}