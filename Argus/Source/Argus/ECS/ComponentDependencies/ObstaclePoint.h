// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class UWorld;

struct ObstaclePoint
{
	FVector2D m_point = FVector2D::ZeroVector;
	FVector2D m_direction = FVector2D::ZeroVector;
	bool m_isConvex = false;

	void DrawDebugObstaclePoint(UWorld* worldPointer) const;
};

class ObstaclePointArray : public TArray<ObstaclePoint>
{
public:
	const ObstaclePoint& GetHead() const;
	const ObstaclePoint& GetTail() const;
	const ObstaclePoint& GetPrevious(int32 index) const;
	const ObstaclePoint& GetNext(int32 index) const;
	void Reverse();
	void AppendOtherToThis(ObstaclePointArray& other);
	void CloseLoop();
};