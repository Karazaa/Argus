// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

namespace ArgusMath
{
	static float Determinant(const FVector2D& topRow, const FVector2D& bottomRow)
	{
		return (topRow.X * bottomRow.Y) - (topRow.Y * bottomRow.X);
	}
}