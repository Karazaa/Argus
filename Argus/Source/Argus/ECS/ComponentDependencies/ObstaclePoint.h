// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

struct ObstaclePoint
{
	FVector2D m_point = FVector2D::ZeroVector;
	FVector2D m_direction = FVector2D::ZeroVector;
	bool m_isConvex = false;
};

class ObstaclePointArray : public TArray<ObstaclePoint>
{
public:
	const ObstaclePoint* GetHead() const;
	const ObstaclePoint* GetTail() const;
	void Reverse();
};