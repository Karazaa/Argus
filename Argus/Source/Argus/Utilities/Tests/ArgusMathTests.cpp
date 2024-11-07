// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusMathExponentialDecaySmootherFloatTest, "Argus.Utilities.ArgusMath.ExponentialDecaySmoother.FloatTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusMathExponentialDecaySmootherFloatTest::RunTest(const FString& Parameters)
{
	const int sampleSize = 60;
	const float framerate = 60.0f;
	const float deltaTime = 1.0f / framerate;
	const float valueTolerance = 0.001f;
	const float value0 = 0.0f;
	const float value1 = 1.0f;
	const float value2 = 10.0f;

	float decayConstant = 10.0f;

	ArgusMath::ExponentialDecaySmoother<float> testValue = ArgusMath::ExponentialDecaySmoother<float>(0.0f, decayConstant);

#pragma region Test that the ExponentialDecaySmoother has the correct initial value.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that it has the correct initial value."),
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

	if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
	{
		UE_LOG(ArgusTestingLog, Display, TEXT("Smoothing from %f to %f"), value0, value1);
	}
	for (int i = 0; i < sampleSize; ++i)
	{
		testValue.SmoothChase(value1, deltaTime);
		if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
		{
			UE_LOG(ArgusTestingLog, Display, TEXT("%s value: %f"), ARGUS_NAMEOF(ExponentialDecaySmoother), testValue.GetValue());
		}
	}

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
			framerate,
			value1
		),
		FMath::IsNearlyEqual(testValue.GetValue(), value1, valueTolerance)
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

	if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
	{
		UE_LOG(ArgusTestingLog, Display, TEXT("Smoothing from %f to %f"), value0, value2);
	}
	for (int i = 0; i < sampleSize; ++i)
	{
		testValue.SmoothChase(value2, deltaTime);
		if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
		{
			UE_LOG(ArgusTestingLog, Display, TEXT("%s value: %f"), ARGUS_NAMEOF(ExponentialDecaySmoother), testValue.GetValue());
		}
	}

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
			framerate,
			value2
		),
		FMath::IsNearlyEqual(testValue.GetValue(), value2, valueTolerance)
	);
#pragma endregion

	if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
	{
		UE_LOG(ArgusTestingLog, Display, TEXT("Smoothing from %f to %f"), value2, value0);
	}
	for (int i = 0; i < sampleSize; ++i)
	{
		testValue.SmoothChase(value0, deltaTime);
		if (CVarEnableVerboseTestLogging.GetValueOnAnyThread())
		{
			UE_LOG(ArgusTestingLog, Display, TEXT("%s value: %f"), ARGUS_NAMEOF(ExponentialDecaySmoother), testValue.GetValue());
		}
	}

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
			framerate,
			value0
		),
		FMath::IsNearlyEqual(testValue.GetValue(), value0, valueTolerance)
	);
#pragma endregion

	return true;
}
#endif //WITH_AUTOMATION_TESTS