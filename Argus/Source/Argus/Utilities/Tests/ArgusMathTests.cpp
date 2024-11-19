// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

namespace ArgusMathTests
{
	static constexpr int   k_sampleSize = 60;
	static constexpr float k_framerate = 60.0f;
	static constexpr float k_deltaTime = 1.0f / k_framerate;
	static constexpr float k_valueTolerance = 0.001f;
}

void ChaseValue(ArgusMath::ExponentialDecaySmoother<float>& smoother, float from, float to)
{
	if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
	{
		ARGUS_LOG(ArgusTestingLog, Display, TEXT("Smoothing from %f to %f"), from, to);
	}
	for (int i = 0; i < ArgusMathTests::k_sampleSize; ++i)
	{
		smoother.SmoothChase(to, ArgusMathTests::k_deltaTime);
		if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
		{
			ARGUS_LOG(ArgusTestingLog, Display, TEXT("%s value: %f"), ARGUS_NAMEOF(ExponentialDecaySmoother<float>), smoother.GetValue());
		}
	}
}

void ChaseValue(ArgusMath::ExponentialDecaySmoother<FVector>& smoother, const FVector& from, const FVector& to)
{
	if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
	{
		ARGUS_LOG
		(
			ArgusTestingLog, Display, TEXT("Smoothing from {%f, %f, %f} to {%f, %f, %f}."),
			from.X, from.Y, from.Z,
			to.X, to.Y, to.Z
		);
	}
	for (int i = 0; i < ArgusMathTests::k_sampleSize; ++i)
	{
		smoother.SmoothChase(to, ArgusMathTests::k_deltaTime);
		const FVector value = smoother.GetValue();
		if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
		{
			ARGUS_LOG
			(
				ArgusTestingLog,
				Display,
				TEXT("%s value: {%f, %f, %f}"), ARGUS_NAMEOF(ExponentialDecaySmoother<FVector>),
				value.X, value.Y, value.Z
			);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusMathExponentialDecaySmootherFloatTest, "Argus.Utilities.ArgusMath.ExponentialDecaySmoother.FloatTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusMathExponentialDecaySmootherFloatTest::RunTest(const FString& Parameters)
{
	const float value0 = 0.0f;
	const float value1 = 1.0f;
	const float value2 = 10.0f;
	const float decayConstant = 10.0f;

	ArgusMath::ExponentialDecaySmoother<float> testValue;

#pragma region Test that the ExponentialDecaySmoother has the correct initial value after using default constructor.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct initial value after calling default constructor."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<float>)
		),
		testValue.GetValue(),
		value0
	);
#pragma endregion

	testValue = ArgusMath::ExponentialDecaySmoother<float>(0.0f, decayConstant);

#pragma region Test that the ExponentialDecaySmoother has the correct initial value after using verbose constructor.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct initial value calling verbose constructor."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<float>)
		),
		testValue.GetValue(),
		value0
	);
#pragma endregion

#pragma region Test that the ExponentialDecaySmoother has the correct initial decay constant.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct initial decay constant."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<float>)
		),
		testValue.GetDecayConstant(),
		decayConstant
	);
#pragma endregion

	ChaseValue(testValue, value0, value1);

#pragma region Test that smoothing from 0 to 1 over 1 second at 60 fps moves value sufficiently close to 1.0.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that smoothing from %f to %f over 1 second at %f fps gets sufficiently close to %f"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<float>),
			value0,
			value1,
			ArgusMathTests::k_framerate,
			value1
		),
		FMath::IsNearlyEqual(testValue.GetValue(), value1, ArgusMathTests::k_valueTolerance)
	);
#pragma endregion

	testValue.Reset(value0);

#pragma region Test that the ExponentialDecaySmoother has the correct value after reset.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, chasing a value, and test resetting back to the initial value."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<float>)
		),
		testValue.GetValue(),
		value0
	);
#pragma endregion

	ChaseValue(testValue, value0, value2);

#pragma region Test that smoothing from 0 to 10 over 1 second at 60 fps moves value sufficiently close to 10.0.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that smoothing from %f to %f over 1 second at %f fps gets sufficiently close to %f"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<float>),
			value0,
			value2,
			ArgusMathTests::k_framerate,
			value2
		),
		FMath::IsNearlyEqual(testValue.GetValue(), value2, ArgusMathTests::k_valueTolerance)
	);
#pragma endregion

	ChaseValue(testValue, value2, value0);

#pragma region Test that smoothing from 10 to 0 over 1 second at 60 fps moves value sufficiently close to 0.0.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that smoothing from %f to %f over 1 second at %f fps gets sufficiently close to %f"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<float>),
			value2,
			value0,
			ArgusMathTests::k_framerate,
			value0
		),
		FMath::IsNearlyEqual(testValue.GetValue(), value0, ArgusMathTests::k_valueTolerance)
	);
#pragma endregion

	testValue.Reset(value2);
	testValue.ResetZero();

#pragma region Test that the ExponentialDecaySmoother has the correct value after calling ResetZero.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct value after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>),
			ARGUS_NAMEOF(testValue.ResetZero)
		),
		testValue.GetValue(),
		value0
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusMathExponentialDecaySmootherFVectorTest, "Argus.Utilities.ArgusMath.ExponentialDecaySmoother.FVectorTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusMathExponentialDecaySmootherFVectorTest::RunTest(const FString& Parameters)
{
	const FVector vector0 = FVector::ZeroVector;
	const FVector vector1 = FVector(100.0f, 50.0f, 80.0f);
	const FVector vector2 = FVector(-1000.0f, 1.0f, -500.0f);
	const float decayConstant = 15.0f;

	ArgusMath::ExponentialDecaySmoother<FVector> testValue;

#pragma region Test that the ExponentialDecaySmoother has the correct initial value after calling default constructor.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct initial value after calling default constructor."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>)
		),
		testValue.GetValue(),
		vector0
	);
#pragma endregion

	testValue = ArgusMath::ExponentialDecaySmoother<FVector>(vector0, decayConstant);

#pragma region Test that the ExponentialDecaySmoother has the correct initial value after calling verbose constructor.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct initial value after calling verbose constructor."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>)
		),
		testValue.GetValue(),
		vector0
	);
#pragma endregion

	ChaseValue(testValue, vector0, vector1);

#pragma region Test that smoothing from vec0 to vec1 over 1 second at 60 fps moves value sufficiently close to vec1.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that smoothing from {%f, %f, %f} to {%f, %f, %f} over 1 second at %f fps gets sufficiently close to target."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>),
			vector0.X, vector0.Y, vector0.Z,
			vector1.X, vector1.Y, vector1.Z,
			ArgusMathTests::k_framerate
		),
		testValue.GetValue().Equals(vector1, ArgusMathTests::k_valueTolerance)
	);
#pragma endregion

	testValue.Reset(vector0);

#pragma region Test that the ExponentialDecaySmoother has the correct value after reset.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, chasing a value, and test resetting back to the initial value."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>)
		),
		testValue.GetValue().Equals(vector0, ArgusMathTests::k_valueTolerance)
	);
#pragma endregion

	ChaseValue(testValue, vector0, vector2);

#pragma region Test that smoothing from vec0 to vec2 over 1 second at 60 fps moves value sufficiently close to vec1.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that smoothing from {%f, %f, %f} to {%f, %f, %f} over 1 second at %f fps gets sufficiently close to target."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>),
			vector0.X, vector0.Y, vector0.Z,
			vector2.X, vector2.Y, vector2.Z,
			ArgusMathTests::k_framerate
		),
		testValue.GetValue().Equals(vector2, ArgusMathTests::k_valueTolerance)
	);
#pragma endregion

	ChaseValue(testValue, vector2, vector0);

#pragma region Test that smoothing from vec2 to vec0 over 1 second at 60 fps moves value sufficiently close to vec1.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that smoothing from {%f, %f, %f} to {%f, %f, %f} over 1 second at %f fps gets sufficiently close to target."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>),
			vector2.X, vector2.Y, vector2.Z,
			vector0.X, vector0.Y, vector0.Z,
			ArgusMathTests::k_framerate
		),
		testValue.GetValue().Equals(vector0, ArgusMathTests::k_valueTolerance)
	);
#pragma endregion

	testValue.Reset(vector2);
	testValue.ResetZero();

#pragma region Test that the ExponentialDecaySmoother has the correct value after calling ResetZero.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct value after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusMath::ExponentialDecaySmoother<FVector>),
			ARGUS_NAMEOF(testValue.ResetZero)
		),
		testValue.GetValue(),
		vector0
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS