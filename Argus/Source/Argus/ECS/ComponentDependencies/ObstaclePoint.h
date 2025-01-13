// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

struct ObstaclePoint
{
	FVector2D m_point = FVector2D::ZeroVector;
	FVector2D m_direction = FVector2D::ZeroVector;
	bool m_isConvex = false;
};