// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusLogging.h"
#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Serialization/Archive.h"

class FArchive;

namespace ArgusMath
{
	float SafeDivide(float numerator, float denominator, float fallbackValue = 0.0f);
	FVector2D SafeDivide(FVector2D numerator, float denominator, FVector2D fallbackValue = FVector2D::ZeroVector);
	FVector SafeDivide(FVector numerator, float denominator, FVector fallbackValue = FVector::ZeroVector);

	float Determinant(const FVector2D& topRow, const FVector2D& bottomRow);
	float AmountLeftOf(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint);

	// Unreal being left handed hurts my soul. Defining two functions here simply for code readability. 
	FVector2D ToCartesianVector2(const FVector2D& vectorToConvert);
	FVector2D ToUnrealVector2(const FVector2D& vectorToConvert);
	FVector ToCartesianVector(const FVector& vectorToConvert);
	FVector ToUnrealVector(const FVector& vectorToConvert);

	bool IsLeftOfCartesian(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint, float radius = 0.0f);
	bool IsLeftOfUnreal(const FVector2D& lineSegmentPoint0, const FVector2D& lineSegmentPoint1, const FVector2D& evaluationPoint, float radius = 0.0f);

	bool DoLineSegmentsIntersectCartesian(const FVector2D& lineSegmentAPoint0, const FVector2D& lineSegmentAPoint1, const FVector2D& lineSegmentBPoint0, const FVector2D& lineSegmentBPoint1);
	bool DoLineSegmentsIntersectUnreal(const FVector2D& lineSegmentAPoint0, const FVector2D& lineSegmentAPoint1, const FVector2D& lineSegmentBPoint0, const FVector2D& lineSegmentBPoint1);

	bool GetLineSegmentIntersectionCartesian(const FVector2D& lineSegmentAPoint0, const FVector2D& lineSegmentAPoint1, const FVector2D& lineSegmentBPoint0, const FVector2D& lineSegmentBPoint1, FVector2D& outIntersectionPoint);
	bool GetRayToPlaneIntersection(const FVector& raySourceLocation, const FVector& rayDirection, const FVector& planeLocation, const FVector& planeNormal, FVector& outIntersection);

	float GetNormalizedZeroToTwoPi(float angle);
	FVector GetDirectionFromYaw(float yaw);
	float GetYawFromDirection(const FVector& direction);
	float GetUEYawDegreesFromYaw(float yaw);

	int FColorAsBGRA(const FColor& color);
	bool IsNearlyEqual(const FVector2D& vector0, const FVector2D& vector1);

	FVector2D Average(const FVector2D& vector0, const FVector2D& vector1);

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

	template<>
	inline uint8 GetZero<uint8>()
	{
		return 0u;
	}

	template<>
	inline uint16 GetZero<uint16>()
	{
		return 0u;
	}

	template<>
	inline int32 GetZero<int32>()
	{
		return 0;
	}

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

	template <typename T, uint8 SmoothingOrder = 1u>
	class ExponentialDecaySmoother
	{
	public:
		static float GetDefaultDecayConstant()
		{
			return (k_maxDecayConstant - k_minDecayConstant) / 2.0f;
		}

		ExponentialDecaySmoother()
		{
			Reset(GetZero<T>());
			m_decayConstant = GetDefaultDecayConstant();
		}

		ExponentialDecaySmoother(float decayConstant)
		{
			Reset(GetZero<T>());
			m_decayConstant = FMath::Clamp(decayConstant, k_minDecayConstant, k_maxDecayConstant);
			m_smoothingSpeedMod = 0.0f;
		}

		ExponentialDecaySmoother(float decayConstant, float smoothingSpeedMod)
		{
			Reset(GetZero<T>());
			m_decayConstant = FMath::Clamp(decayConstant, k_minDecayConstant, k_maxDecayConstant);
			m_smoothingSpeedMod = smoothingSpeedMod;
		}

		ExponentialDecaySmoother(const T& initialValue, float decayConstant, float smoothingSpeedMod)
		{
			Reset(initialValue);
			m_decayConstant = FMath::Clamp(decayConstant, k_minDecayConstant, k_maxDecayConstant);
			m_smoothingSpeedMod = smoothingSpeedMod;
		}

		T GetValue() const
		{
			return m_currentValues[SmoothingOrder - 1u];
		}

		float GetDecayConstant() const
		{
			return m_decayConstant;
		}

		void Reset(T initialValue)
		{
			for (uint8 i = 0u; i < SmoothingOrder; ++i)
			{
				m_currentValues[i] = initialValue;
			}
		}

		void ResetZero()
		{
			Reset(GetZero<T>());
		}

		void SmoothChase(const T& targetValue, float deltaTime)
		{
			const float smoothingSpeed = 1.0f + (m_smoothingSpeedMod / 200.0f);
			SmoothChase(m_currentValues[0], targetValue, m_decayConstant, smoothingSpeed, deltaTime);
			for (uint8 i = 1u; i < SmoothingOrder; ++i)
			{
				SmoothChase(m_currentValues[i], m_currentValues[i - 1], m_decayConstant, smoothingSpeed, deltaTime);
			}
		}

		static void SmoothChase(T& value, const T& targetValue, float decayConstant, float smoothingSpeed, float deltaTime)
		{
			decayConstant = FMath::Clamp(decayConstant, k_minDecayConstant, k_maxDecayConstant);
			value = targetValue + ((value - targetValue) * FMath::Exp(-decayConstant * deltaTime) * smoothingSpeed);
		}

		void Serialize(FArchive& archive)
		{
			for (uint8 i = 0; i < SmoothingOrder; ++i)
			{
				archive << m_currentValues[i];
			}
			archive << m_decayConstant;
			archive << m_smoothingSpeedMod;
		}

	private:
		static constexpr float k_minDecayConstant = 1.0f;
		static constexpr float k_maxDecayConstant = 30.0f;

		T m_currentValues[SmoothingOrder];
		float m_decayConstant = k_minDecayConstant;
		float m_smoothingSpeedMod = 0.0f;
	};

	template<typename T>
	class SecondOrderSystemSmoother
	{
	public:
		SecondOrderSystemSmoother()
		{
			ResetZero();
			ComputeConstants(1.0f, 1.0f, 0.0f);
		}

		SecondOrderSystemSmoother(float f, float z, float r)
		{
			ResetZero();
			ComputeConstants(f, z, r);
		}

		SecondOrderSystemSmoother(float f, float z, float r, const T& initialValue)
		{
			Reset(initialValue);
			ComputeConstants(f, z, r);
		}

		T GetValue () const { return m_currentValue; }

		void Reset(const T& initialValue)
		{
			m_previousTarget = initialValue;
			m_currentValue = initialValue;
			m_currentVelocity = GetZero<T>();
		}

		void ResetZero()
		{
			Reset(GetZero<T>());
		}

		void SmoothChase(const T& targetValue, float deltaTime)
		{
			if (FMath::IsNearlyZero(deltaTime))
			{
				return;
			}

			T targetVelocity = (targetValue - m_previousTarget) / deltaTime;
			m_previousTarget = targetValue;

			const float k2Stable = FMath::Max3(m_k2, FMath::Square(deltaTime) * 0.5f, deltaTime * m_k1 * 0.5f);
			m_currentValue = m_currentValue + (m_currentVelocity * deltaTime);
			m_currentVelocity = m_currentVelocity + ((targetValue + (targetVelocity * m_k3) - m_currentValue - (m_currentVelocity * m_k1)) * SafeDivide(deltaTime, k2Stable));
		}

		void Serialize(FArchive& archive)
		{
			archive << m_previousTarget;
			archive << m_currentValue;
			archive << m_currentVelocity;
			archive << m_k1;
			archive << m_k2;
			archive << m_k3;
		}

	private:
		void ComputeConstants(float f, float z, float r)
		{
			const float fpi = UE_PI * f;
			const float fpi2 = 2.0f * fpi;
			m_k1 = SafeDivide(z, fpi);
			m_k2 = SafeDivide(1.0f, FMath::Square(fpi2));
			m_k3 = SafeDivide(r * z, fpi2);
		}

		T m_previousTarget;
		T m_currentValue;
		T m_currentVelocity;
		float m_k1 = 0.0f;
		float m_k2 = 0.0f;
		float m_k3 = 0.0f;
	};
}