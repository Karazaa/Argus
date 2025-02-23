// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"

namespace ArgusMath
{
	template<typename T>
	static T GetZero()
	{
		return T();
	}

	template<>
	inline float GetZero<float>()
	{
		return 0.0f;
	}

	template<>
	inline FVector2D GetZero<FVector2D>()
	{
		return FVector2D::ZeroVector;
	}

	template<>
	inline FVector GetZero<FVector>()
	{
		return FVector::ZeroVector;
	}

	template <typename T>
	class ExponentialDecaySmoother
	{
	public:
		static float GetDefaultDecayConstant()
		{
			(k_maxDecayConstant - k_minDecayConstant) / 2.0f;
		}

		ExponentialDecaySmoother()
		{
			m_currentValue = GetZero<T>();
			m_decayConstant = GetDefaultDecayConstant();
		}

		ExponentialDecaySmoother(float decayConstant)
		{
			m_currentValue = GetZero<T>();
			m_decayConstant = FMath::Clamp(decayConstant, k_minDecayConstant, k_maxDecayConstant);
		}

		ExponentialDecaySmoother(T initialValue, float decayConstant) : m_currentValue(initialValue)
		{
			m_decayConstant = FMath::Clamp(decayConstant, k_minDecayConstant, k_maxDecayConstant);
		}

		T GetValue() const
		{
			return m_currentValue;
		}

		float GetDecayConstant() const
		{
			return m_decayConstant;
		}

		void Reset(T initialValue)
		{
			m_currentValue = initialValue;
		}

		void ResetZero()
		{
			m_currentValue = GetZero<T>();
		}

		void SmoothChase(const T& targetValue, float deltaTime)
		{
			m_currentValue = targetValue + ((m_currentValue - targetValue) * FMath::Exp(-m_decayConstant * deltaTime));
		}

	private:
		static constexpr float k_minDecayConstant = 1.0f;
		static constexpr float k_maxDecayConstant = 30.0f;

		T m_currentValue;
		float m_decayConstant = k_minDecayConstant;
	};

	template <typename T>
	static T SafeDivide(T numerator, float denominator, T fallbackValue)
	{
		if (FMath::IsNearlyZero(denominator))
		{
			ARGUS_LOG(ArgusUtilitiesLog, Warning, TEXT("[%s] Division by zero occured!"), ARGUS_FUNCNAME);
			return fallbackValue;
		}

		return numerator / denominator;
	}

	static float SafeDivide(float numerator, float denominator, float fallbackValue = 0.0f)
	{
		return SafeDivide<float>(numerator, denominator, fallbackValue);
	}

	static FVector2D SafeDivide(FVector2D numerator, float denominator, FVector2D fallbackValue = FVector2D::ZeroVector)
	{
		return SafeDivide<FVector2D>(numerator, denominator, fallbackValue);
	}

	static FVector SafeDivide(FVector numerator, float denominator, FVector fallbackValue = FVector::ZeroVector)
	{
		return SafeDivide<FVector>(numerator, denominator, fallbackValue);
	}

	static float Determinant(const FVector2D& topRow, const FVector2D& bottomRow)
	{
		return (topRow.X * bottomRow.Y) - (topRow.Y * bottomRow.X);
	}

	static float AmountLeftOf(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint)
	{
		return Determinant(lineSegmentPoint0 - evaluationPoint, lineSegmentPoint1 - lineSegmentPoint0);
	}

	// Unreal being left handed hurts my soul. Defining two functions here simply for code readability. 
	static FVector2D ToCartesianVector2(const FVector2D& vectorToConvert)
	{
		FVector2D output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	static FVector2D ToUnrealVector2(const FVector2D& vectorToConvert)
	{
		FVector2D output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	static FVector ToCartesianVector(const FVector& vectorToConvert)
	{
		FVector output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	static FVector ToUnrealVector(const FVector& vectorToConvert)
	{
		FVector output = vectorToConvert;
		output.Y *= -1.0f;
		return output;
	}

	static bool IsLeftOfCartesian(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint)
	{
		return AmountLeftOf(lineSegmentPoint0, lineSegmentPoint1, evaluationPoint) > 0.0f;
	}

	static bool IsLeftOfUnreal(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint)
	{
		return AmountLeftOf(ToCartesianVector2(lineSegmentPoint0), ToCartesianVector2(lineSegmentPoint1), ToCartesianVector2(evaluationPoint)) > 0.0f;
	}

	static FVector GetDirectionFromYaw(float yaw)
	{
		return FVector(FMath::Cos(yaw), -FMath::Sin(yaw), 0.0f);
	}

	static float GetYawFromDirection(const FVector& direction)
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
}