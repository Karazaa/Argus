// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePoint.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

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