// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusMath.h"

namespace ArgusMath
{
	float SafeDivide(float numerator, float denominator, float fallbackValue)
	{
		return SafeDivide<float>(numerator, denominator, fallbackValue);
	}

	FVector2D SafeDivide(FVector2D numerator, float denominator, FVector2D fallbackValue)
	{
		return SafeDivide<FVector2D>(numerator, denominator, fallbackValue);
	}

	FVector SafeDivide(FVector numerator, float denominator, FVector fallbackValue)
	{
		return SafeDivide<FVector>(numerator, denominator, fallbackValue);
	}

	float Determinant(const FVector2D& topRow, const FVector2D& bottomRow)
	{
		return (topRow.X * bottomRow.Y) - (topRow.Y * bottomRow.X);
	}

	float AmountLeftOf(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint)
	{
		return Determinant(lineSegmentPoint0 - evaluationPoint, lineSegmentPoint1 - lineSegmentPoint0);
	}

	FVector2D ToCartesianVector2(const FVector2D& vectorToConvert)
	{
		FVector2D output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	FVector2D ToUnrealVector2(const FVector2D& vectorToConvert)
	{
		FVector2D output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	FVector ToCartesianVector(const FVector& vectorToConvert)
	{
		FVector output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	FVector ToUnrealVector(const FVector& vectorToConvert)
	{
		FVector output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	bool IsLeftOfCartesian(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint, float radius)
	{
		if (radius == 0.0f)
		{
			return AmountLeftOf(lineSegmentPoint0, lineSegmentPoint1, evaluationPoint) > 0.0f;
		}

		// Get a normal vector pointing rightwards from the evaluation line segment
		FVector2D radiusAdjustmentDirection = lineSegmentPoint1 - lineSegmentPoint0;
		std::swap(radiusAdjustmentDirection.X, radiusAdjustmentDirection.Y);
		radiusAdjustmentDirection.Y *= -1.0f;

		return AmountLeftOf(lineSegmentPoint0, lineSegmentPoint1, (radiusAdjustmentDirection.GetSafeNormal() * radius) + evaluationPoint) > 0.0f;
	}

	bool IsLeftOfUnreal(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint, float radius)
	{
		return IsLeftOfCartesian(ToCartesianVector2(lineSegmentPoint0), ToCartesianVector2(lineSegmentPoint1), ToCartesianVector2(evaluationPoint), radius);
	}

	bool DoLineSegmentsIntersectCartesian(const FVector2D& lineSegmentAPoint0, const FVector2D& lineSegmentAPoint1, const FVector2D& lineSegmentBPoint0, const FVector2D& lineSegmentBPoint1)
	{
		const bool differA = IsLeftOfCartesian(lineSegmentAPoint0, lineSegmentAPoint1, lineSegmentBPoint0) != IsLeftOfCartesian(lineSegmentAPoint0, lineSegmentAPoint1, lineSegmentBPoint1);
		const bool differB = IsLeftOfCartesian(lineSegmentBPoint0, lineSegmentBPoint1, lineSegmentAPoint0) != IsLeftOfCartesian(lineSegmentBPoint0, lineSegmentBPoint1, lineSegmentAPoint1);
		return differA && differB;
	}

	bool DoLineSegmentsIntersectUnreal(const FVector2D& lineSegmentAPoint0, const FVector2D& lineSegmentAPoint1, const FVector2D& lineSegmentBPoint0, const FVector2D& lineSegmentBPoint1)
	{
		return DoLineSegmentsIntersectCartesian(ToCartesianVector2(lineSegmentAPoint0), ToCartesianVector2(lineSegmentAPoint1), ToCartesianVector2(lineSegmentBPoint0), ToCartesianVector2(lineSegmentBPoint1));
	}

	bool GetLineSegmentIntersectionCartesian(const FVector2D& lineSegmentAPoint0, const FVector2D& lineSegmentAPoint1, const FVector2D& lineSegmentBPoint0, const FVector2D& lineSegmentBPoint1, FVector2D& outIntersectionPoint)
	{
		const FVector2D xDifference = FVector2D(lineSegmentAPoint0.X - lineSegmentAPoint1.X, lineSegmentBPoint0.X - lineSegmentBPoint1.X);
		const FVector2D yDifference = FVector2D(lineSegmentAPoint0.Y - lineSegmentAPoint1.Y, lineSegmentBPoint0.Y - lineSegmentBPoint1.Y);
		const float determinant = Determinant(xDifference, yDifference);

		if (FMath::IsNearlyZero(determinant))
		{
			return false;
		}

		const FVector2D dValue = FVector2D(Determinant(lineSegmentAPoint0, lineSegmentAPoint1), Determinant(lineSegmentBPoint0, lineSegmentBPoint1));
		outIntersectionPoint.X = Determinant(dValue, xDifference) / determinant;
		outIntersectionPoint.Y = Determinant(dValue, yDifference) / determinant;

		return DoLineSegmentsIntersectCartesian(lineSegmentAPoint0, lineSegmentAPoint1, lineSegmentBPoint0, lineSegmentBPoint1);
	}


	float GetNormalizedZeroToTwoPi(float angle)
	{
		float modulo = FMath::Fmod(angle, UE_TWO_PI);
		if (modulo < 0.0f)
		{
			modulo += UE_TWO_PI;
		}

		return modulo;
	}

	FVector GetDirectionFromYaw(float yaw)
	{
		return FVector(FMath::Cos(yaw), -FMath::Sin(yaw), 0.0f);
	}

	float GetYawFromDirection(const FVector& direction)
	{
		FVector normalizedDirection = direction;
		normalizedDirection.Z = 0.0f;
		normalizedDirection.Normalize();
		const float arcsine = FMath::Asin(-normalizedDirection.Y);
		const float arccosine = FMath::Acos(normalizedDirection.X);

		if (arcsine >= 0.0f)
		{
			return arccosine;
		}

		if (arccosine < UE_HALF_PI)
		{
			return UE_TWO_PI + arcsine;
		}

		return UE_PI - arcsine;
	}

	float GetUEYawDegreesFromYaw(float yaw)
	{
		const float yawDegreeOffset = FMath::RadiansToDegrees(GetNormalizedZeroToTwoPi(yaw));
		return FMath::IsNearlyZero(yawDegreeOffset) ? 0.0f : 360.0f - yawDegreeOffset;
	}

	int FColorAsBGRA(const FColor& color)
	{
		int output = 0;
		output |= (color.B << 24);
		output |= (color.G << 16);
		output |= (color.R << 8);
		output |= (color.A);

		return output;
	}

	bool IsNearlyEqual(const FVector2D& vector0, const FVector2D& vector1)
	{
		return FMath::IsNearlyEqual(vector0.X, vector1.X) && FMath::IsNearlyEqual(vector0.Y, vector1.Y);
	}

	FVector2D Average(const FVector2D& vector0, const FVector2D& vector1)
	{
		return (vector0 + vector1) * 0.5f;
	}
}