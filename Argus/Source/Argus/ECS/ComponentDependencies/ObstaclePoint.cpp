// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePoint.h"

const ObstaclePoint* ObstaclePointArray::GetHead() const
{
	if (Num() == 0)
	{
		return nullptr;
	}
	
	return &(GetData()[0]);
}

const ObstaclePoint* ObstaclePointArray::GetTail() const
{
	if (Num() == 0)
	{
		return nullptr;
	}

	return &(GetData()[Num() - 1]);
}

void ObstaclePointArray::Reverse()
{
	const int32 halfObstaclePoints = Num() / 2;
	for (int32 i = 0; i < halfObstaclePoints; ++i)
	{
		Swap(i, Num() - (i + 1));
	}
}