// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "CoreMinimal.h"

class UWorld;

struct ObstaclePoint
{
	FVector2D m_point = FVector2D::ZeroVector;
	FVector2D m_direction = FVector2D::ZeroVector;
	float m_height = 0.0f;
	bool m_isConvex = false;

	FVector2D GetLeftVector() const { return FVector2D(-m_direction.Y, m_direction.X); }
	FVector2D GetRightVector() const { return FVector2D(m_direction.Y, -m_direction.X); }
	void DrawDebugObstaclePoint(UWorld* worldPointer) const;
};

class ObstaclePointArray : public TArray<ObstaclePoint, ArgusContainerAllocator<100u> >
{
public:
	const ObstaclePoint& GetHead() const;
	const ObstaclePoint& GetTail() const;
	const ObstaclePoint& GetPrevious(int32 index) const;
	const ObstaclePoint& GetNext(int32 index) const;

	void FillInBetweenObstaclePoints(const ObstaclePoint& fromPoint, const ObstaclePoint& toPoint, TArray<ObstaclePoint>& outPoints);
	void AddObstaclePointsWithFillIn(const ObstaclePoint& instigatingObstacle, bool addToHead);
	void Reverse();
	void AppendOtherToThis(ObstaclePointArray& other);
	void CloseLoop();
	bool IsPointElevated(int32 index) const;
	bool IsNextPointElevated(int32 index) const;

	float m_floorHeight = 0.0f;
};

class ObstaclesContainer : public TArray<ObstaclePointArray, ArgusContainerAllocator<25u> > {};