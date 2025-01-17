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

	Append(&other[1], other.Num() - 1);
}

void ObstaclePointArray::CloseLoop()
{
	if (GetHead().m_point == GetTail().m_point)
	{
		RemoveAt(Num() - 1, EAllowShrinking::No);
	}
}