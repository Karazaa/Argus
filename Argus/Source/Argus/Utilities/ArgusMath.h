// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

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
	inline FVector GetZero<FVector>()
	{
		return FVector::ZeroVector;
	}

	template <typename T>
	class ExponentialDecaySmoother
	{
	public:
		ExponentialDecaySmoother()
		{
			m_currentValue = GetZero<T>();
			m_decayConstant = (k_maxDecayConstant - k_minDecayConstant) / 2.0f;
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

	static float Determinant(const FVector2D& topRow, const FVector2D& bottomRow)
	{
		return (topRow.X * bottomRow.Y) - (topRow.Y * bottomRow.X);
	}

	static bool IsLeftOfCartesian(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint)
	{
		return Determinant(lineSegmentPoint0 - evaluationPoint, lineSegmentPoint1 - lineSegmentPoint0) > 0.0f;
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
}