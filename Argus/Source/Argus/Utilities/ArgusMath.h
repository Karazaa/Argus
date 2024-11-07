// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

namespace ArgusMath
{
	template <typename T>
	class ExponentialDecaySmoother
	{
	public:
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

		void SmoothChase(const T& targetValue, float deltaTime)
		{
			m_currentValue = targetValue + ((m_currentValue - targetValue) * FMath::Exp(-m_decayConstant * deltaTime));
		}

	private:
		static constexpr float k_minDecayConstant = 1.0f;
		static constexpr float k_maxDecayConstant = 30.0f;

		T m_currentValue;
		float m_decayConstant;
	};

	static float Determinant(const FVector2D& topRow, const FVector2D& bottomRow)
	{
		return (topRow.X * bottomRow.Y) - (topRow.Y * bottomRow.X);
	}
}