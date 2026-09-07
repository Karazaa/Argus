// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "CoreMinimal.h"
#include "ObstaclePoint.generated.h"

class UWorld;

USTRUCT()
struct ARGUS_API FObstaclePoint
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector2D m_point = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	FVector2D m_direction = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	float m_height = 0.0f;

	UPROPERTY(VisibleAnywhere)
	bool m_isConvex = false;

	UPROPERTY(VisibleAnywhere)
	bool m_isAlias = false;

	FVector2D GetLeftVector() const { return FVector2D(-m_direction.Y, m_direction.X); }
	FVector2D GetRightVector() const { return FVector2D(m_direction.Y, -m_direction.X); }

#if !UE_BUILD_SHIPPING
	void DrawDebugObstaclePoint(UWorld* worldPointer, float duration, bool shouldShowText, bool isPointElevated) const;
#endif //!UE_BUILD_SHIPPING
};

USTRUCT()
struct ARGUS_API FObstaclePointArray
{
	GENERATED_BODY()

	const FObstaclePoint& GetHead() const;
	const FObstaclePoint& GetTail() const;
	const int32 GetPreviousIndex(int32 index) const;
	const FObstaclePoint& GetPrevious(int32 index) const;
	const int32 GetPreviousNonAliasIndex(int32 index) const;
	const int32 GetCurrentNonAliasIndex(int32 index) const;
	const FObstaclePoint& GetPreviousNonAlias(int32 index) const;
	const int32 GetNextIndex(int32 index) const;
	const FObstaclePoint& GetNext(int32 index) const;
	const int32 GetNextNonAliasIndex(int32 index) const;
	const FObstaclePoint& GetNextNonAlias(int32 index) const;


	void FillInBetweenObstaclePoints(const FObstaclePoint& fromPoint, const FObstaclePoint& toPoint, TArray<FObstaclePoint>& outPoints);
	void AddObstaclePointsWithFillIn(const FObstaclePoint& instigatingObstacle, bool addToHead);
	void Reverse();
	void AppendOtherToThis(FObstaclePointArray& other);
	void CloseLoop();
	void ConsolidateNearbyPoints();
	bool IsPointElevated(int32 index) const;
	bool IsNextPointElevated(int32 index) const;

	UPROPERTY(VisibleAnywhere)
	TArray<FObstaclePoint> m_obstaclePoints;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector2D> m_fixupDirections;

	UPROPERTY(VisibleAnywhere)
	float m_floorHeight = 0.0f;
};

USTRUCT()
struct ARGUS_API FObstaclesContainer 
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FObstaclePointArray> m_obstacleArrays;
};